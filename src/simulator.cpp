#include <bits/stdc++.h>
#include "random.hpp"
#include "coordinate.hpp"
#include "ant.hpp"
#include "data.hpp"
#include "constants.hpp"

std::string normalizeNumber(const std::string &s) {
    std::string out = s;
    std::replace(out.begin(), out.end(), ',', '.');
    return out;
}

std::vector<Data> readDataFile(const std::string &filename) {
    std::ifstream file(filename);
    std::vector<Data> dataset;

    if (!file.is_open()) {
        throw std::runtime_error("could not open file: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string sx, sy, sg;

        if (!(ss >> sx >> sy >> sg)) continue;

        double x = std::stod(normalizeNumber(sx));
        double y = std::stod(normalizeNumber(sy));
        int group = std::stoi(sg);

        dataset.emplace_back(x, y, group);
    }

    return dataset;
}

void normalizeData(std::vector<Data>& dataset) {
    Data mean, var, st;
    for (Data data : dataset) {
        mean.x += data.x;
        mean.y += data.y;
    }
    mean.x /= dataset.size();
    mean.y /= dataset.size();
    for (Data data : dataset) {
        var.x += (data.x - mean.x) * (data.x - mean.x);
        var.y += (data.y - mean.y) * (data.y - mean.y);
    }
    st.x = std::sqrt(var.x / dataset.size());
    st.y = std::sqrt(var.y / dataset.size());

    for (Data& data : dataset) {
        data.x = (data.x - mean.x) / st.x;
        data.y = (data.y - mean.y) / st.y;
    }
}


struct World {
    int N, M;
    int ant_number, item_number;
    std::vector<std::vector<int>> ant_map, filled_map, carrying_map;
    std::vector<std::vector<Data>> data_map;
    std::vector<Ant> ants;

    World(int _N, int _M, int _ants, std::vector<Data> items) 
        : N(_N), M(_M), ant_number(_ants), item_number(items.size()) {

        filled_map.assign(N, std::vector<int>(M, false)); 
        ant_map.assign(N, std::vector<int>(M, 0)); 
        carrying_map.assign(N, std::vector<int>(M, 0)); 
        data_map.assign(N, std::vector<Data>(M, Data())); 
        ants.reserve(ant_number);

        for (Coordinate cord : random_coordinates(N, M, ant_number)) {
            ant_map[cord.x][cord.y] += 1;
            ants.emplace_back(cord, RADIUS);
        }
        int i = 0;
        for (Coordinate cord : random_coordinates(N, M, item_number)) {
            filled_map[cord.x][cord.y] = true;
            data_map[cord.x][cord.y] = items[i++];
        }
        print();
    }

    Coordinate wrap(int x, int y) {
        int _x = x, _y = y;
        _x = _x < 0 ? N - 1 : _x;
        _x = _x >= N ? 0 : _x;

        _y = _y < 0 ? M - 1 : _y;
        _y = _y >= M ? 0 : _y;

        return Coordinate(_x, _y);
    }

    Coordinate random_step(int x, int y) {
        int _x = x, _y = y;
        _x += uniform(-1, 1);
        _y += uniform(-1, 1);

        return wrap(_x, _y);
    }

#ifdef LOCAL_DEBUG
    void print(int step_number = -1) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                if (filled_map[i][j] && ant_map[i][j]) {
                    std::cerr << data_map[i][j].group;
                } else if (filled_map[i][j]) {
                    std::cerr << data_map[i][j].group;
                } else if (ant_map[i][j]) {
                    std::cerr << '.';
                } else {
                    std::cerr << '.';
                }
            }
            std::cerr << std::endl;
        }
        std::cerr << std::endl;
        std::cerr << std::endl;
    }
#else
    void print(int step_number = -1) {
        std::string fname;
        if (step_number >= 0) fname = "res/step_" + std::to_string(step_number) + ".pgm";
        else fname = "res/step_first.pgm";

        std::ofstream fout(fname, std::ios::binary);
        if (!fout) return;

        // Header P6: width height maxval
        fout << "P6\n" << M << " " << N << "\n255\n";

        // 0 -> ant (sem carregar)
        // 1 -> data1
        // 2 -> data2
        // 3 -> data3
        // 4 -> data4
        // 5 -> empty
        std::vector<std::array<uint8_t,3>> palette = {
            {  0,   0,   0}, // 0 ant   -> preto
            {239,  71, 111}, // 1 data1 -> rosa/vermelho
            {255, 209, 102}, // 2 data2 -> amarelo quente
            {  6, 214, 160}, // 3 data3 -> verde água
            { 17, 138, 178}, // 4 data4 -> azul petróleo
            {255, 255, 255}  // 5 empty -> branco
        };

        std::array<uint8_t,3> carrier_color = {0, 0, 0}; // preto tb
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                const std::array<uint8_t,3>* colPtr = nullptr;

                if (carrying_map[i][j]) assert(ant_map[i][j]);

                if (filled_map[i][j]) {
                    int group = data_map[i][j].group;
                    // garantir índice válido (se necessário, clipe)
                    colPtr = &palette[group];
                } else if (ant_map[i][j]) {
                    if (carrying_map[i][j]) colPtr = &carrier_color;
                    else colPtr = &palette[0];
                } else {
                    colPtr = &palette[5];
                }

                fout.write(reinterpret_cast<const char*>(colPtr->data()), 3);
            }
        }
        fout.close();
    }
#endif

    void do_action(Ant& ant) {
        // formiga carregando nao pode pegar item
        // formiga nao carregando so pode pegar o item se estiver na cordenada dela
        if (ant.carrying == filled_map[ant.cord.x][ant.cord.y]) return;

        std::vector<Data> v;
        for (int i = -ant.radius; i <= ant.radius; i++) {
            for (int j = -ant.radius; j <= ant.radius; j++) {
                Coordinate cur = wrap(ant.cord.x + i, ant.cord.y + j);
                const Data data = data_map[cur.x][cur.y];
                if (filled_map[cur.x][cur.y]) {
                    assert(data.group != 0);
                    v.emplace_back(data);
                } else {
                    assert(data.group == 0);
                }
            }
        }

        const Data data = data_map[ant.cord.x][ant.cord.y];
        if (ant.action(v, data)) {
            filled_map[ant.cord.x][ant.cord.y] ^= 1;
            ant.carrying ^= 1;
            if (ant.carrying) {
                carrying_map[ant.cord.x][ant.cord.y] += 1;
                assert(data.group != 0);
                ant.data = data;
                data_map[ant.cord.x][ant.cord.y] = Data();
            }
            else {
                assert(data.group == 0);
                carrying_map[ant.cord.x][ant.cord.y] -= 1;
                data_map[ant.cord.x][ant.cord.y] = ant.data;
            }
        }
    }

    void step() {
        for (Ant& ant : ants) {
            do_action(ant);
        }

        for (Ant& ant : ants) {
            int x = ant.cord.x, y = ant.cord.y;

            if (ant.carrying) carrying_map[x][y] -= 1;
            ant_map[x][y] -= 1;

            ant.cord = random_step(x, y);
            x = ant.cord.x, y = ant.cord.y;

            ant_map[x][y] += 1;
            if (ant.carrying) carrying_map[x][y] += 1;
        }

    }

    Coordinate first_not_filled(Coordinate cord) {
        std::queue<Coordinate> q;
        q.emplace(cord);
        std::vector<Coordinate> to;
        std::vector vis(N, std::vector<bool>(M, false));
        while (q.size()) {
            Coordinate now = q.front();
            q.pop();
            vis[now.x][now.y] = true;
            if (!filled_map[now.x][now.y]) {
                return now;
            }
            for (int i = -RADIUS; i <= RADIUS; i++) {
                for (int j = -RADIUS; j <= RADIUS; j++) {
                    Coordinate cur = wrap(now.x + i, now.y + j);
                    if (!vis[cur.x][cur.y]) {
                        vis[cur.x][cur.y] = true;
                        q.emplace(cur.x, cur.y);
                    }
                }
            }
        }
        assert(false);
        return Coordinate();
    }

    void endstep() {
        for (Ant& ant : ants) {
            if (ant.carrying) {
                Coordinate now = first_not_filled(ant.cord);
                ant_map[ant.cord.x][ant.cord.y] -= 1;
                carrying_map[ant.cord.x][ant.cord.y] -= 1;
                ant_map[now.x][now.y] += 1;
                filled_map[now.x][now.y] = 1;
            }
        }
    }

    void simulate(int STEPS, int print_every) {
        for (int i = 0; i <= STEPS; i++) {
            step();
            if (i % print_every == 0) {
                print(i);
            }
        }

        endstep();
        print(STEPS);
    }
};

signed main() {

    std::vector<Data> dataset = readDataFile("res/Square1-DataSet-400itens.txt");
    //normalizeData(dataset);
    World world(64, 64, 150, dataset);
    world.simulate(20000000, 100000);

    //World world(15, 15, 20, 50);
    //world.simulate(100000, 1000);
}

