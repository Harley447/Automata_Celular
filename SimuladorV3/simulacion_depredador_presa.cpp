#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <functional>
#include <string>
#include <cstdint>
#include <filesystem>
#include <thread>
#include <chrono>
#include <set>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>

// ==========================================
// CONFIGURACION GENERAL (Valores por defecto)
// ==========================================
bool VISUALIZAR = true;
int GENERACIONES_POR_SEGUNDO = 60; // Acelerado para experimentos
bool GUARDAR_INSTANTANEAS = true;
std::set<int> INSTANTANEAS_GENERACIONES = {1, 10, 50, 100, 500, 999};
std::string CARPETA_INSTANTANEAS = "instantaneas";

int N = 500, M = 500;
int presasIni = 50000;
int depIni = 5000;
int T = 1000;

// Tipos de vecindario
enum class Vecindario { VON_NEUMANN, MOORE };

// Estados posibles de una celda
enum class Estado { VACIA, PRESA, DEPREDADOR };

// Estructura para cada celda/individuo
struct Celda {
    Estado estado = Estado::VACIA;
    int edad = 0;
    int energia = 0;
    bool yaActuo = false; // Bandera para evitar el doble turno
};

// Coordenada auxiliar
struct Pos { int x, y; };

// Firmas para reglas configurables
using ReglaReproduccion = std::function<bool(const Celda&)>;
using ReglaMuerte = std::function<bool(const Celda&)>;
using ReglaAlimentacion = std::function<int(const Celda&)>;

// ==========================================
// MODELO PRINCIPAL (MOTOR DE SIMULACION)
// ==========================================
class ModeloDepredadorPresa {
private:
    int N, M;
    std::vector<Celda> grilla;
    Vecindario tipoVecindario;
    std::mt19937 rng;

    ReglaReproduccion reprPresa;
    ReglaMuerte muertePresa;
    ReglaReproduccion reprDepredador;
    ReglaMuerte muerteDepredador;
    ReglaAlimentacion reglaAlimentacion;

    inline int getIdx(int x, int y) const { return y * N + x; }

public:
    ModeloDepredadorPresa(int n, int m, unsigned int semilla)
        : N(n), M(m), grilla(n * m), tipoVecindario(Vecindario::MOORE), rng(semilla) {
        reglaAlimentacion = [](const Celda&) { return 4; };
    }

    void setVecindario(Vecindario v) { tipoVecindario = v; }

    void setReglasPresa(ReglaReproduccion r, ReglaMuerte m) {
        reprPresa = r; muertePresa = m;
    }

    void setReglasDepredador(ReglaReproduccion r, ReglaMuerte m, ReglaAlimentacion a) {
        reprDepredador = r; muerteDepredador = m; reglaAlimentacion = a;
    }

    void inicializar(int numPresas, int numDepredadores, int energiaInicialDep) {
        std::fill(grilla.begin(), grilla.end(), Celda{Estado::VACIA, 0, 0, false});

        std::vector<int> indices(N * M);
        for (int i = 0; i < N * M; ++i) indices[i] = i;
        std::shuffle(indices.begin(), indices.end(), rng);

        for (int i = 0; i < numPresas; ++i)
            grilla[indices[i]] = Celda{Estado::PRESA, 0, 0, false};
        for (int i = numPresas; i < numPresas + numDepredadores; ++i)
            grilla[indices[i]] = Celda{Estado::DEPREDADOR, 0, energiaInicialDep, false};
    }

    std::vector<Pos> obtenerVecinos(int x, int y) {
        std::vector<Pos> vecinos;
        int dx[] = {-1, 1, 0, 0, -1, -1, 1, 1};
        int dy[] = {0, 0, -1, 1, -1, 1, -1, 1};
        int max_dir = (tipoVecindario == Vecindario::VON_NEUMANN) ? 4 : 8;

        for (int i = 0; i < max_dir; ++i) {
            int nx = (x + dx[i] + N) % N; // Frontera periódica (Toroidal)
            int ny = (y + dy[i] + M) % M;
            vecinos.push_back({nx, ny});
        }
        return vecinos;
    }

    void simularGeneracion() {
        // 1. Reiniciar la bandera de actuación para todos
        for (auto& c : grilla) {
            c.yaActuo = false;
        }

        // 2. Recolectar posiciones actuales
        std::vector<Pos> individuos;
        individuos.reserve(N * M);
        for (int y = 0; y < M; ++y) {
            for (int x = 0; x < N; ++x) {
                if (grilla[getIdx(x, y)].estado != Estado::VACIA) {
                    individuos.push_back({x, y});
                }
            }
        }

        // 3. Barajar el orden de actualización asíncrona
        std::shuffle(individuos.begin(), individuos.end(), rng);

        // 4. Ejecutar turnos
        for (const auto& pos : individuos) {
            int idx = getIdx(pos.x, pos.y);
            // Si la celda quedó vacía o el individuo ya actuó
            if (grilla[idx].estado == Estado::VACIA || grilla[idx].yaActuo) continue;

            if (grilla[idx].estado == Estado::PRESA)
                turnoPresa(pos.x, pos.y);
            else if (grilla[idx].estado == Estado::DEPREDADOR)
                turnoDepredador(pos.x, pos.y);
        }
    }

    void turnoPresa(int x, int y) {
        int idx = getIdx(x, y);
        Celda presa = grilla[idx];
        presa.edad++;
        presa.yaActuo = true; // Marcar como que ya operó en este turno

        auto vecinos = obtenerVecinos(x, y);
        std::vector<Pos> vacias;
        for (auto v : vecinos)
            if (grilla[getIdx(v.x, v.y)].estado == Estado::VACIA)
                vacias.push_back(v);

        Pos posActual = {x, y};
        Pos posOriginal = {x, y};

        // Regla de movimiento
        if (!vacias.empty()) {
            std::uniform_int_distribution<int> dist(0, vacias.size() - 1);
            posActual = vacias[dist(rng)];
            grilla[getIdx(posActual.x, posActual.y)] = presa;
            grilla[getIdx(posOriginal.x, posOriginal.y)].estado = Estado::VACIA;
        } else {
            grilla[idx] = presa; // Se actualiza su edad in-situ
        }

        // Regla de reproducción
        if (reprPresa(grilla[getIdx(posActual.x, posActual.y)])) {
            if (posActual.x != posOriginal.x || posActual.y != posOriginal.y) {
                // Deja una cría en la posición original. Nace con yaActuo = true
                grilla[getIdx(posOriginal.x, posOriginal.y)] = Celda{Estado::PRESA, 0, 0, true};
            }
            grilla[getIdx(posActual.x, posActual.y)].edad = 0;
        }

        // Regla de muerte
        if (muertePresa(grilla[getIdx(posActual.x, posActual.y)])) {
            grilla[getIdx(posActual.x, posActual.y)].estado = Estado::VACIA;
        }
    }

    void turnoDepredador(int x, int y) {
        int idx = getIdx(x, y);
        Celda dep = grilla[idx];
        dep.edad++;
        dep.yaActuo = true; // Marcar como que ya operó en este turno

        auto vecinos = obtenerVecinos(x, y);
        std::vector<Pos> presas, vacias;
        for (auto v : vecinos) {
            Estado e = grilla[getIdx(v.x, v.y)].estado;
            if (e == Estado::PRESA) presas.push_back(v);
            else if (e == Estado::VACIA) vacias.push_back(v);
        }

        Pos posActual = {x, y};
        Pos posOriginal = {x, y};

        // Regla de movimiento y alimentación
        if (!presas.empty()) {
            std::uniform_int_distribution<int> dist(0, presas.size() - 1);
            posActual = presas[dist(rng)];
            dep.energia += reglaAlimentacion(grilla[getIdx(posActual.x, posActual.y)]);
            grilla[getIdx(posActual.x, posActual.y)] = dep;
            grilla[getIdx(posOriginal.x, posOriginal.y)].estado = Estado::VACIA;
        }
        else if (!vacias.empty()) {
            std::uniform_int_distribution<int> dist(0, vacias.size() - 1);
            posActual = vacias[dist(rng)];
            dep.energia--;
            grilla[getIdx(posActual.x, posActual.y)] = dep;
            grilla[getIdx(posOriginal.x, posOriginal.y)].estado = Estado::VACIA;
        }
        else {
            dep.energia--;
            grilla[idx] = dep;
        }

        // Regla de muerte y reproducción
        if (muerteDepredador(grilla[getIdx(posActual.x, posActual.y)])) {
            grilla[getIdx(posActual.x, posActual.y)].estado = Estado::VACIA;
        }
        else if (reprDepredador(grilla[getIdx(posActual.x, posActual.y)])) {
            if (posActual.x != posOriginal.x || posActual.y != posOriginal.y) {
                // Deja una cría con yaActuo = true
                grilla[getIdx(posOriginal.x, posOriginal.y)] = Celda{Estado::DEPREDADOR, 0, 4, true};
            }
            grilla[getIdx(posActual.x, posActual.y)].edad = 0;
        }
    }

    void contarPoblacion(int& presas, int& depredadores) {
        presas = depredadores = 0;
        for (const auto& c : grilla) {
            if (c.estado == Estado::PRESA) presas++;
            else if (c.estado == Estado::DEPREDADOR) depredadores++;
        }
    }

    const std::vector<Celda>& getGrilla() const { return grilla; }
};

// ==========================================
// VARIANTES DE REGLAS (PUNTOS DE EXTENSION)
// ==========================================

bool reproPorEdad(const Celda& c, int umbral) { return c.edad >= umbral; }
bool reproProbabilistica(const Celda& c, double prob, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng) < prob;
}
bool muerteNunca(const Celda& c) { return false; }
bool muerteInanicion(const Celda& c) { return c.energia <= 0; }
bool muerteVejez(const Celda& c, int maxEdad) { return c.edad >= maxEdad; }
bool muerteMixta(const Celda& c, int maxEdad) { return c.energia <= 0 || c.edad >= maxEdad; }

// ==========================================
// UTILIDADES GRAFICAS
// ==========================================
void guardarInstantanea(const sf::Texture& textura, int generacion, const std::string& subcarpeta) {
    std::string ruta = CARPETA_INSTANTANEAS + "/" + subcarpeta;
    std::filesystem::create_directories(ruta);
    sf::Image imagen = textura.copyToImage();
    std::string nombreArchivo = ruta + "/gen_" + std::to_string(generacion) + ".png";
    if (!imagen.saveToFile(nombreArchivo)) {
        std::cerr << "Error al guardar instantanea en: " << nombreArchivo << "\n";
    } else {
        std::cout << "Instantanea guardada: " << nombreArchivo << "\n";
    }
}

// NUEVA FIRMA: Añadido parametro tituloTexto
void guardarGraficaPNG(const std::vector<int>& gen, const std::vector<int>& presas,
                       const std::vector<int>& depredadores, const std::string& nombreArchivo,
                       const std::string& tituloTexto) {
    const int ancho = 800, alto = 600;
    sf::RenderTexture renderTexture(sf::Vector2u(ancho, alto));
    renderTexture.clear(sf::Color::White);

    sf::Font fuente;
    bool fuenteCargada = fuente.openFromFile("C:/Windows/Fonts/arial.ttf");
    if (!fuenteCargada) fuenteCargada = fuente.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    sf::VertexArray ejeX(sf::PrimitiveType::Lines, 2);
    ejeX[0].position = sf::Vector2f(50, alto - 50); ejeX[1].position = sf::Vector2f(ancho - 20, alto - 50);
    ejeX[0].color = sf::Color::Black; ejeX[1].color = sf::Color::Black;

    sf::VertexArray ejeY(sf::PrimitiveType::Lines, 2);
    ejeY[0].position = sf::Vector2f(50, 20); ejeY[1].position = sf::Vector2f(50, alto - 50);
    ejeY[0].color = sf::Color::Black; ejeY[1].color = sf::Color::Black;

    renderTexture.draw(ejeX);
    renderTexture.draw(ejeY);

    if (fuenteCargada) {
        // Uso del titulo dinámico
        sf::Text titulo(fuente, tituloTexto, 24);
        titulo.setFillColor(sf::Color::Black);
        titulo.setPosition(sf::Vector2f(ancho / 2.0f - titulo.getLocalBounds().size.x / 2.0f, 10));
        renderTexture.draw(titulo);

        sf::Text leyendaP(fuente, "Presas", 14);
        leyendaP.setFillColor(sf::Color::Green);
        leyendaP.setPosition(sf::Vector2f(ancho - 120, 50));
        renderTexture.draw(leyendaP);

        sf::Text leyendaD(fuente, "Depredadores", 14);
        leyendaD.setFillColor(sf::Color::Red);
        leyendaD.setPosition(sf::Vector2f(ancho - 120, 70));
        renderTexture.draw(leyendaD);
    }

    if (!gen.empty()) {
        int maxPob = *std::max_element(presas.begin(), presas.end());
        maxPob = std::max(maxPob, *std::max_element(depredadores.begin(), depredadores.end()));
        if (maxPob == 0) maxPob = 1;

        sf::VertexArray curvaPresas(sf::PrimitiveType::LineStrip, gen.size());
        sf::VertexArray curvaDepred(sf::PrimitiveType::LineStrip, gen.size());

        for (size_t i = 0; i < gen.size(); ++i) {
            float x = 50 + (float)gen[i] / gen.back() * (ancho - 70);
            float yPresas = alto - 50 - (float)presas[i] / maxPob * (alto - 70);
            float yDepred = alto - 50 - (float)depredadores[i] / maxPob * (alto - 70);
            curvaPresas[i].position = sf::Vector2f(x, yPresas); curvaPresas[i].color = sf::Color::Green;
            curvaDepred[i].position = sf::Vector2f(x, yDepred); curvaDepred[i].color = sf::Color::Red;
        }
        renderTexture.draw(curvaPresas);
        renderTexture.draw(curvaDepred);
    }

    renderTexture.display();
    sf::Texture textura = renderTexture.getTexture();
    sf::Image imagen = textura.copyToImage();
    
    if (!imagen.saveToFile(nombreArchivo)) {
        std::cerr << "Error al guardar grafica en: " << nombreArchivo << "\n";
    }
}

// ==========================================
// PROGRAMA PRINCIPAL
// ==========================================
int main(int argc, char* argv[]) {
    // Parámetros por defecto
    unsigned int semilla = 42;
    std::string str_vecindario = "moore";
    double prob_repro_presa = 0.2;
    int umbral_repro_dep = 12;
    int energia_ini_dep = 4;
    std::string identificador_exp = "default";
    bool sin_ventana = false;

    // Parseo de argumentos por linea de comandos
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--semilla" && i + 1 < argc) semilla = std::stoul(argv[++i]);
        else if (arg == "--vecindario" && i + 1 < argc) str_vecindario = argv[++i];
        else if (arg == "--prob_presa" && i + 1 < argc) prob_repro_presa = std::stod(argv[++i]);
        else if (arg == "--umbral_dep" && i + 1 < argc) umbral_repro_dep = std::stoi(argv[++i]);
        else if (arg == "--energia_ini" && i + 1 < argc) energia_ini_dep = std::stoi(argv[++i]);
        else if (arg == "--exp_id" && i + 1 < argc) identificador_exp = argv[++i];
        else if (arg == "--headless") sin_ventana = true;
    }

    if (sin_ventana) VISUALIZAR = false;
    
    std::mt19937 rng_reglas(semilla);
    ModeloDepredadorPresa modelo(N, M, semilla);
    
    if (str_vecindario == "von_neumann") modelo.setVecindario(Vecindario::VON_NEUMANN);
    else modelo.setVecindario(Vecindario::MOORE);

    modelo.setReglasPresa(
        [&rng_reglas, prob_repro_presa](const Celda& c) { return reproProbabilistica(c, prob_repro_presa, rng_reglas); },
        [](const Celda& c) { return muerteVejez(c, 15); }
    );

    modelo.setReglasDepredador(
        [umbral_repro_dep](const Celda& c) { return reproPorEdad(c, umbral_repro_dep); },
        [](const Celda& c) { return muerteMixta(c, 20); },
        [](const Celda&) { return 2; }
    );

    modelo.inicializar(presasIni, depIni, energia_ini_dep);

    std::unique_ptr<sf::RenderWindow> window;
    std::unique_ptr<sf::Texture> textura;
    std::unique_ptr<sf::Sprite> sprite;
    std::vector<std::uint8_t> pixeles(N * M * 4, 0);

    if (VISUALIZAR) {
        window = std::make_unique<sf::RenderWindow>(sf::VideoMode({800, 800}), "Simulacion Exp: " + identificador_exp);
        window->setFramerateLimit(static_cast<unsigned int>(GENERACIONES_POR_SEGUNDO));
        textura = std::make_unique<sf::Texture>(sf::Vector2u(N, M));
        sprite = std::make_unique<sf::Sprite>(*textura);
        
        sprite->setScale(sf::Vector2f(800.0f / N, 800.0f / M));
    } else {
        textura = std::make_unique<sf::Texture>(sf::Vector2u(N, M));
    }

    std::vector<int> histGen, histPresas, histDep;
    bool corriendo = true;
    int generacion = 0;

    std::cout << "Iniciando experimento: " << identificador_exp << "\n"
              << "Semilla: " << semilla << " | Vecindario: " << str_vecindario << "\n";

    while (corriendo && generacion < T) {
        if (VISUALIZAR) {
            while (auto event = window->pollEvent()) {
                if (event->is<sf::Event::Closed>()) corriendo = false;
            }
        }
        if (!corriendo) break;

        modelo.simularGeneracion();
        generacion++;

        if (VISUALIZAR || (GUARDAR_INSTANTANEAS && INSTANTANEAS_GENERACIONES.count(generacion))) {
            const auto& grilla = modelo.getGrilla();
            for (int y = 0; y < M; ++y) {
                for (int x = 0; x < N; ++x) {
                    int idx = y * N + x;
                    int pIdx = idx * 4;
                    if (grilla[idx].estado == Estado::VACIA) {
                        pixeles[pIdx] = pixeles[pIdx+1] = pixeles[pIdx+2] = 0;
                    } else if (grilla[idx].estado == Estado::PRESA) {
                        pixeles[pIdx] = pixeles[pIdx+2] = 0; pixeles[pIdx+1] = 255;
                    } else {
                        pixeles[pIdx+1] = pixeles[pIdx+2] = 0; pixeles[pIdx] = 255;
                    }
                    pixeles[pIdx+3] = 255;
                }
            }
            textura->update(pixeles.data());

            if (VISUALIZAR) {
                window->clear();
                window->draw(*sprite);
                window->display();
            }

            if (GUARDAR_INSTANTANEAS && INSTANTANEAS_GENERACIONES.count(generacion)) {
                guardarInstantanea(*textura, generacion, identificador_exp);
            }
        }

        int p, d;
        modelo.contarPoblacion(p, d);
        histGen.push_back(generacion); histPresas.push_back(p); histDep.push_back(d);

        // IMPRESIÓN ACTUALIZADA: Cada 50 generaciones
        if (generacion % 50 == 0) {
            std::cout << "Gen " << generacion << " | Presas: " << p << " | Depredadores: " << d << "\n";
        }

        if (p == 0 && d == 0) {
            std::cout << "Extincion total en gen " << generacion << "\n";
            break;
        }
    }

    if (window) window->close();
    std::string nomGrafica = "grafica_" + identificador_exp + ".png";
    
    // LLAMADA ACTUALIZADA: Pasando el identificador del experimento como título
    guardarGraficaPNG(histGen, histPresas, histDep, nomGrafica, "Prueba: " + identificador_exp);
    std::cout << "Grafica guardada: " << nomGrafica << "\n";

    return 0;
}