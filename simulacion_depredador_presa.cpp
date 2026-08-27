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
// CONFIGURACION GENERAL
// ==========================================
const bool VISUALIZAR = true;
const int GENERACIONES_POR_SEGUNDO = 5;
const bool GUARDAR_INSTANTANEAS = true;
const std::set<int> INSTANTANEAS_GENERACIONES = {1, 10, 20, 30, 50};
const std::string CARPETA_INSTANTANEAS = "instantaneas";

// Parametros de la simulacion
const int N = 500, M = 500;
const int presasIni = 50000;
const int depIni = 5000;
const int energiaInicialDep = 4;
const int T = 1000;
const unsigned int SEMILLA = 42;

// Tipos de vecindario
enum class Vecindario { VON_NEUMANN, MOORE };

// Estados posibles de una celda
enum class Estado { VACIA, PRESA, DEPREDADOR };

// Estructura para cada celda/individuo
struct Celda {
    Estado estado = Estado::VACIA;
    int edad = 0;
    int energia = 0;
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
        std::fill(grilla.begin(), grilla.end(), Celda{Estado::VACIA, 0, 0});

        std::vector<int> indices(N * M);
        for (int i = 0; i < N * M; ++i) indices[i] = i;
        std::shuffle(indices.begin(), indices.end(), rng);

        for (int i = 0; i < numPresas; ++i)
            grilla[indices[i]] = Celda{Estado::PRESA, 0, 0};
        for (int i = numPresas; i < numPresas + numDepredadores; ++i)
            grilla[indices[i]] = Celda{Estado::DEPREDADOR, 0, energiaInicialDep};
    }

    std::vector<Pos> obtenerVecinos(int x, int y) {
        std::vector<Pos> vecinos;
        int dx[] = {-1, 1, 0, 0, -1, -1, 1, 1};
        int dy[] = {0, 0, -1, 1, -1, 1, -1, 1};
        int max_dir = (tipoVecindario == Vecindario::VON_NEUMANN) ? 4 : 8;

        for (int i = 0; i < max_dir; ++i) {
            int nx = (x + dx[i] + N) % N;
            int ny = (y + dy[i] + M) % M;
            vecinos.push_back({nx, ny});
        }
        return vecinos;
    }

    void simularGeneracion() {
        std::vector<Pos> individuos;
        for (int y = 0; y < M; ++y)
            for (int x = 0; x < N; ++x)
                if (grilla[getIdx(x, y)].estado != Estado::VACIA)
                    individuos.push_back({x, y});

        std::shuffle(individuos.begin(), individuos.end(), rng);

        for (const auto& pos : individuos) {
            int idx = getIdx(pos.x, pos.y);
            if (grilla[idx].estado == Estado::VACIA) continue;

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
        grilla[idx] = presa;

        auto vecinos = obtenerVecinos(x, y);
        std::vector<Pos> vacias;
        for (auto v : vecinos)
            if (grilla[getIdx(v.x, v.y)].estado == Estado::VACIA)
                vacias.push_back(v);

        Pos posActual = {x, y};
        Pos posOriginal = {x, y};

        if (!vacias.empty()) {
            std::uniform_int_distribution<int> dist(0, vacias.size() - 1);
            posActual = vacias[dist(rng)];
            grilla[getIdx(posActual.x, posActual.y)] = presa;
            grilla[getIdx(posOriginal.x, posOriginal.y)].estado = Estado::VACIA;
        }

        if (reprPresa(grilla[getIdx(posActual.x, posActual.y)])) {
            if (posActual.x != posOriginal.x || posActual.y != posOriginal.y)
                grilla[getIdx(posOriginal.x, posOriginal.y)] = Celda{Estado::PRESA, 0, 0};
            grilla[getIdx(posActual.x, posActual.y)].edad = 0;
        }

        if (muertePresa(grilla[getIdx(posActual.x, posActual.y)]))
            grilla[getIdx(posActual.x, posActual.y)].estado = Estado::VACIA;
    }

    void turnoDepredador(int x, int y) {
        int idx = getIdx(x, y);
        Celda dep = grilla[idx];
        dep.edad++;
        grilla[idx] = dep;

        auto vecinos = obtenerVecinos(x, y);
        std::vector<Pos> presas, vacias;
        for (auto v : vecinos) {
            Estado e = grilla[getIdx(v.x, v.y)].estado;
            if (e == Estado::PRESA) presas.push_back(v);
            else if (e == Estado::VACIA) vacias.push_back(v);
        }

        Pos posActual = {x, y};
        Pos posOriginal = {x, y};

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

        if (muerteDepredador(grilla[getIdx(posActual.x, posActual.y)])) {
            grilla[getIdx(posActual.x, posActual.y)].estado = Estado::VACIA;
        }
        else if (reprDepredador(grilla[getIdx(posActual.x, posActual.y)])) {
            if (posActual.x != posOriginal.x || posActual.y != posOriginal.y)
                grilla[getIdx(posOriginal.x, posOriginal.y)] = Celda{Estado::DEPREDADOR, 0, 4};
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
    int getAncho() const { return N; }
    int getAlto() const { return M; }
};

// ==========================================
// VARIANTES DE REGLAS (PUNTOS DE EXTENSION)
// ==========================================

bool reproPorEdad(const Celda& c, int umbral) {
    return c.edad >= umbral;
}

bool reproProbabilistica(const Celda& c, double prob, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng) < prob;
}

bool muerteNunca(const Celda& c) { return false; }
bool muerteInanicion(const Celda& c) { return c.energia <= 0; }
bool muerteVejez(const Celda& c, int maxEdad) { return c.edad >= maxEdad; }
bool muerteProbabilistica(const Celda& c, double prob, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng) < prob;
}
bool muerteMixta(const Celda& c, int maxEdad) {
    return c.energia <= 0 || c.edad >= maxEdad;
}

// ==========================================
// FUNCION PARA GUARDAR INSTANTANEAS PNG
// ==========================================
void guardarInstantanea(const sf::Texture& textura, int generacion) {
    std::filesystem::create_directories(CARPETA_INSTANTANEAS);
    sf::Image imagen = textura.copyToImage();
    std::string nombreArchivo = CARPETA_INSTANTANEAS + "/gen_" + std::to_string(generacion) + ".png";
    if (imagen.saveToFile(nombreArchivo)) {
        std::cout << "Instantanea guardada: " << nombreArchivo << "\n";
    } else {
        std::cerr << "Error al guardar instantanea: " << nombreArchivo << "\n";
    }
}

// ==========================================
// FUNCION PARA GUARDAR GRAFICA PNG
// ==========================================
void guardarGraficaPNG(const std::vector<int>& gen,
                       const std::vector<int>& presas,
                       const std::vector<int>& depredadores,
                       const std::string& nombreArchivo) {
    const int ancho = 800, alto = 600;
    // Usar constructor en lugar de create
    sf::RenderTexture renderTexture(sf::Vector2u(ancho, alto));
    renderTexture.clear(sf::Color::White);

    // Ejes
    sf::VertexArray ejeX(sf::PrimitiveType::Lines, 2);
    ejeX[0].position = sf::Vector2f(50, alto - 50);
    ejeX[1].position = sf::Vector2f(ancho - 20, alto - 50);
    ejeX[0].color = sf::Color::Black;
    ejeX[1].color = sf::Color::Black;

    sf::VertexArray ejeY(sf::PrimitiveType::Lines, 2);
    ejeY[0].position = sf::Vector2f(50, 20);
    ejeY[1].position = sf::Vector2f(50, alto - 50);
    ejeY[0].color = sf::Color::Black;
    ejeY[1].color = sf::Color::Black;

    renderTexture.draw(ejeX);
    renderTexture.draw(ejeY);

    if (!gen.empty()) {
        int maxPob = *std::max_element(presas.begin(), presas.end());
        maxPob = std::max(maxPob, *std::max_element(depredadores.begin(), depredadores.end()));

        sf::VertexArray curvaPresas(sf::PrimitiveType::LineStrip, gen.size());
        sf::VertexArray curvaDepred(sf::PrimitiveType::LineStrip, gen.size());

        for (size_t i = 0; i < gen.size(); ++i) {
            float x = 50 + (float)gen[i] / gen.back() * (ancho - 70);
            float yPresas = alto - 50 - (float)presas[i] / maxPob * (alto - 70);
            float yDepred = alto - 50 - (float)depredadores[i] / maxPob * (alto - 70);
            curvaPresas[i].position = sf::Vector2f(x, yPresas);
            curvaPresas[i].color = sf::Color::Green;
            curvaDepred[i].position = sf::Vector2f(x, yDepred);
            curvaDepred[i].color = sf::Color::Red;
        }

        renderTexture.draw(curvaPresas);
        renderTexture.draw(curvaDepred);
    }

    renderTexture.display();
    sf::Texture textura = renderTexture.getTexture();
    sf::Image imagen = textura.copyToImage();
    if (imagen.saveToFile(nombreArchivo)) {
        std::cout << "Grafica guardada en " << nombreArchivo << "\n";
    } else {
        std::cerr << "Error al guardar grafica\n";
    }
}

// ==========================================
// PROGRAMA PRINCIPAL
// ==========================================
int main() {
    std::mt19937 rng_reglas(SEMILLA);

    ModeloDepredadorPresa modelo(N, M, SEMILLA);
    modelo.setVecindario(Vecindario::MOORE);

    modelo.setReglasPresa(
        [&rng_reglas](const Celda& c) { return reproProbabilistica(c, 0.2, rng_reglas); },
        [](const Celda& c) { return muerteVejez(c, 15); }
    );

    modelo.setReglasDepredador(
        [](const Celda& c) { return reproPorEdad(c, 12); },
        [](const Celda& c) { return muerteMixta(c, 20); },
        [](const Celda&) { return 2; }
    );

    modelo.inicializar(presasIni, depIni, energiaInicialDep);

    std::unique_ptr<sf::RenderWindow> window;
    std::unique_ptr<sf::Texture> textura;
    std::unique_ptr<sf::Sprite> sprite;
    std::vector<std::uint8_t> pixeles(N * M * 4, 0);

    if (VISUALIZAR) {
        const int ventanaAncho = 1000;
        const int ventanaAlto = 1000;
        window = std::make_unique<sf::RenderWindow>(
            sf::VideoMode({ventanaAncho, ventanaAlto}),
            "Simulacion Depredador-Presa (Toroidal 500x500)"
        );
        window->setFramerateLimit(60);

        textura = std::make_unique<sf::Texture>(sf::Vector2u(N, M));
        sprite = std::make_unique<sf::Sprite>(*textura);
        sf::Vector2f escala(static_cast<float>(ventanaAncho) / N,
                            static_cast<float>(ventanaAlto) / M);
        sprite->setScale(escala);
    }

    std::vector<int> historialGeneraciones, historialPresas, historialDepredadores;

    std::cout << "Iniciando simulacion (N=" << N << ", M=" << M << ")\n";
    if (VISUALIZAR)
        std::cout << "Ventana abierta. Velocidad: " << GENERACIONES_POR_SEGUNDO
                  << " gen/seg. Cierra la ventana para terminar.\n";
    else
        std::cout << "Modo sin ventana (solo datos).\n";

    bool corriendo = true;
    int generacion = 0;

    using namespace std::chrono;
    auto tiempoPorGeneracion = milliseconds(1000 / GENERACIONES_POR_SEGUNDO);

    while (corriendo && generacion < T) {
        auto inicio = steady_clock::now();

        if (VISUALIZAR) {
            while (auto event = window->pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    corriendo = false;
                }
            }
            if (!corriendo) break;
        }

        modelo.simularGeneracion();
        generacion++;

        if (VISUALIZAR) {
            const auto& grilla = modelo.getGrilla();
            for (int y = 0; y < M; ++y) {
                for (int x = 0; x < N; ++x) {
                    int idx = y * N + x;
                    int pixelIdx = idx * 4;
                    switch (grilla[idx].estado) {
                        case Estado::VACIA:
                            pixeles[pixelIdx] = 0;
                            pixeles[pixelIdx+1] = 0;
                            pixeles[pixelIdx+2] = 0;
                            break;
                        case Estado::PRESA:
                            pixeles[pixelIdx] = 0;
                            pixeles[pixelIdx+1] = 255;
                            pixeles[pixelIdx+2] = 0;
                            break;
                        case Estado::DEPREDADOR:
                            pixeles[pixelIdx] = 255;
                            pixeles[pixelIdx+1] = 0;
                            pixeles[pixelIdx+2] = 0;
                            break;
                    }
                    pixeles[pixelIdx+3] = 255;
                }
            }
            textura->update(pixeles.data());
            window->clear();
            window->draw(*sprite);
            window->display();
        }

        if (GUARDAR_INSTANTANEAS && INSTANTANEAS_GENERACIONES.count(generacion)) {
            if (textura) {
                guardarInstantanea(*textura, generacion);
            } else {
                sf::Texture tempTextura(sf::Vector2u(N, M));
                std::vector<std::uint8_t> tempPixeles(N * M * 4, 0);
                const auto& grilla = modelo.getGrilla();
                for (int y = 0; y < M; ++y) {
                    for (int x = 0; x < N; ++x) {
                        int idx = y * N + x;
                        int pixelIdx = idx * 4;
                        switch (grilla[idx].estado) {
                            case Estado::VACIA:
                                tempPixeles[pixelIdx] = 0;
                                tempPixeles[pixelIdx+1] = 0;
                                tempPixeles[pixelIdx+2] = 0;
                                break;
                            case Estado::PRESA:
                                tempPixeles[pixelIdx] = 0;
                                tempPixeles[pixelIdx+1] = 255;
                                tempPixeles[pixelIdx+2] = 0;
                                break;
                            case Estado::DEPREDADOR:
                                tempPixeles[pixelIdx] = 255;
                                tempPixeles[pixelIdx+1] = 0;
                                tempPixeles[pixelIdx+2] = 0;
                                break;
                        }
                        tempPixeles[pixelIdx+3] = 255;
                    }
                }
                tempTextura.update(tempPixeles.data());
                guardarInstantanea(tempTextura, generacion);
            }
        }

        int p, d;
        modelo.contarPoblacion(p, d);

        historialGeneraciones.push_back(generacion);
        historialPresas.push_back(p);
        historialDepredadores.push_back(d);

        if (generacion % 50 == 0) {
            std::cout << "Gen " << generacion << " | Presas: " << p
                      << " | Depredadores: " << d << "\n";
        }

        if (p == 0 && d == 0) {
            std::cout << "Extincion total en gen " << generacion << "\n";
            break;
        }

        if (VISUALIZAR && corriendo) {
            auto fin = steady_clock::now();
            auto transcurrido = duration_cast<milliseconds>(fin - inicio);
            if (transcurrido < tiempoPorGeneracion) {
                std::this_thread::sleep_for(tiempoPorGeneracion - transcurrido);
            }
        }
    }

    if (window) window->close();
    std::cout << "Simulacion terminada.\n";
    if (GUARDAR_INSTANTANEAS)
        std::cout << "Instantaneas guardadas en carpeta '" << CARPETA_INSTANTANEAS << "'\n";

    guardarGraficaPNG(historialGeneraciones, historialPresas, historialDepredadores, "grafica_poblacion.png");

    return 0;
}