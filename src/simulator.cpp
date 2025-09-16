#include <bits/stdc++.h>
#include "random.hpp"
#include "coordinate.hpp"
#include "ant.hpp"
#include "data.hpp"

const int RADIUS = 1;

struct World {
    int N, M;
    int ant_number, item_number;
    std::vector<std::vector<int>> ant_map, filled_map, carrying_map;
    std::vector<std::vector<Data>> data_map;
    std::vector<Ant> ants;

    World(int _N, int _M, int _ants, int _items) 
        : N(_N), M(_M), ant_number(_ants), item_number(_items) {

        filled_map.assign(N, std::vector<int>(M, false)); 
        ant_map.assign(N, std::vector<int>(M, 0)); 
        carrying_map.assign(N, std::vector<int>(M, 0)); 
        data_map.assign(N, std::vector<Data>(M, Data())); 
        ants.reserve(ant_number);

        for (Coordinate cord : random_coordinates(N, M, ant_number)) {
            ant_map[cord.x][cord.y] += 1;
            ants.emplace_back(cord, RADIUS);
        }
        for (Coordinate cord : random_coordinates(N, M, item_number)) {
            int a = uniform(1,20);
            if(uniform(1,2) == 2) a *= -1;
            int b = uniform(1,20);
            if(uniform(1,2) == 2) b *= -1;
            int g = 0;
            if(a > 0 && b > 0) {
                g = 1;
            }
            else if(a < 0 && b > 0) {
                g = 2;
            }
            else if(a < 0 && b < 0) {
                g = 3;
            }
            else if(a > 0 && b < 0) {
                g = 4;
            }
            filled_map[cord.x][cord.y] = true;
            data_map[cord.x][cord.y] = Data(a,b,g);
        }
        print();
    }

    Coordinate wrap(int x, int y) {
        int _x = x, _y = y;
        _x = _x < 0 ? N - 1 : _x;;
        _x = _x >= N ? 0 : _x;

        _y = _y < 0 ? M - 1 : _y;
        _y = _y >= M ? 0 : _y;

        return Coordinate(_x, _y);
    }

    Coordinate random_step(int x, int y) {
        int _x = x, _y = y;
        _x += uniform(-1, 1);
        _y += uniform(-1 , 1);

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
        // escreve arquivo PGM binário (P5). Dimensões: M=width, N=height
        std::string fname;
        if (step_number >= 0) fname = "res/step_" + std::to_string(step_number) + ".pgm";
        else fname = "res/step_last.pgm";

        std::ofstream fout(fname, std::ios::binary);
        if (!fout) return; // falha ao abrir arquivo -> não faz nada

        // Header P5: width height maxval
        fout << "P5\n" << M << " " << N << "\n255\n";

        // Mapeamento de valores (um único byte por pixel):
        // both (item + ant) -> 0   (preto)
        // item only         ->  80 (escuro)
        // ant carrying      -> 180
        // ant only          -> 220 (claro)
        // empty             -> 255 (branco)

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                unsigned char v;

                if (carrying_map[i][j]) assert(ant_map[i][j]);

                if (filled_map[i][j] && ant_map[i][j]) v = 0;
                else if (filled_map[i][j]) v = 80;
                else if (carrying_map[i][j]) v = 180;
                else if (ant_map[i][j]) v = 220;
                else v = 255;
                fout.write(reinterpret_cast<char*>(&v), 1);
            }
        }
        fout.close();
    }
#endif

    void do_action(Ant& ant) {
        int cell_cnt = 0;
        std::vector<Data> v;
        for (int i = -ant.radius; i <= ant.radius; i++) {
            for (int j = -ant.radius; j <= ant.radius; j++) {
                Coordinate cur = wrap(ant.cord.x + i, ant.cord.y + j);
                cell_cnt++;
                if (filled_map[cur.x][cur.y]) {
                    v.emplace_back(data_map[cur.x][cur.y]);
                }
            }
        }

        if (ant.action(v, cell_cnt)) {
            if (ant.carrying ^ filled_map[ant.cord.x][ant.cord.y]) {
                filled_map[ant.cord.x][ant.cord.y] ^= 1;
                ant.carrying ^= 1;
                if (ant.carrying) {
                    carrying_map[ant.cord.x][ant.cord.y] += 1;
                    data_map[ant.cord.x][ant.cord.y] = ant.data;
                }
                else {
                    carrying_map[ant.cord.x][ant.cord.y] -= 1;
                    ant.data = data_map[ant.cord.x][ant.cord.y];
                    data_map[ant.cord.x][ant.cord.y] = Data();
                }
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
                to.emplace_back(now);
                continue;
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

        int rand = uniform(0, int(to.size() - 1));
        return to[rand];
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
    }
};

signed main() {
    //World world(4, 4, 1, 1);
    //world.simulate(2, 1);

    World world(40, 40, 100, 500);
    world.simulate(100000, 1000);

    //World world(5, 5, 4, 10);
    //world.simulate(10000, 100);
}

