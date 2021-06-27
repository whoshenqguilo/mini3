#include <iostream>
#include <fstream>
#include <array>
#include <set>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>

using namespace std;
const int SIZE = 8;
const int INF_VALUE = 1000000;

struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(int x, int y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

// 每回合
class OthelloBoard {
private:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    array<array<int, SIZE>, SIZE> board;
    vector<Point> next_valid_spots;
    array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;
private:
    // 下一手是黑子還白子下
    int get_next_player(int player) const {
        return 3 - player;
    }
    // 點是否在棋盤內
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    // 這個點是什麼棋(黑白空)
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    // 設置這格是什麼棋
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    // 
    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    // 判斷這步棋能不能下(判斷棋布合法)
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    // 更新棋盤
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
public:
    OthelloBoard(array<array<int, SIZE>, SIZE> board, vector<Point> next_valid_spots, int cur_player)
    :board(board), next_valid_spots(next_valid_spots), cur_player(cur_player) {
        disc_count[EMPTY] = 0;
        disc_count[BLACK] = 0;
        disc_count[WHITE] = 0;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if(board[i][j] == BLACK)
                    disc_count[BLACK] ++;
                else if(board[i][j] == WHITE)
                    disc_count[WHITE] ++;
            }
        }
        done = false;
        winner = -1;
    }
    OthelloBoard(const OthelloBoard & round){
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = round.board[i][j];
            }
        }
        for(int i = 0; i < round.next_valid_spots.size(); i++){
            Point p = round.next_valid_spots[i];
            next_valid_spots.push_back(p);
        }
        cur_player = round.cur_player;
        disc_count[EMPTY] = round.disc_count[EMPTY];
        disc_count[BLACK] = round.disc_count[BLACK];
        disc_count[WHITE] = round.disc_count[WHITE];
        done = false;
        winner = -1;
    }
    // 下這步棋後對手可下的地方(已經排除掉不合法棋步)
    vector<Point> get_valid_spots() const {
        vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    // 下這步棋
    bool put_disc(Point p) {
        if(!is_spot_valid(p)) {
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        return true;
    }
    //
    vector<Point> get_cur_next_valid_spots(){
        return next_valid_spots;
    }
};

class AI {
private:
    // state value
    int state_value[8][8] = {
        100,-50, 5, 3, 3, 5,-50, 100,
        -50,-100, 5, 1, 1, 5,-100, -50,
         5, 5, 5, 1, 1, 5, 5,  5,
         3, 0, 1, 3, 3, 1, 0,  3,
         3, 0, 1, 3, 3, 1, 0,  3,
         5, 5, 5, 1, 1, 5, 5,  5,
        -50,-100, 5, 1, 1, 5,-100, -50,
        100,-50, 5, 3, 3, 5,-50, 100,
    };
    int limit_depth = 4;
    // informations
    OthelloBoard cur_round;
public:
    AI(OthelloBoard & cur_round):cur_round(cur_round) {}
    // state value
    int evaluation(Point point){
        int spot_value = 0;
        spot_value += state_value[point.x][point.y];
        return spot_value;
    }
    // minimax recursion ()
    // this round(), choice point, depth, opponenet or me, alpha, beta
    int minimax(OthelloBoard & round, Point choice_point, int depth, bool player_type, int alpha, int beta){
        if(depth == limit_depth){
            return evaluation(choice_point);
        }
        OthelloBoard next_round(round);
        next_round.put_disc(choice_point);
        if(player_type){
            int best_value = -INF_VALUE;
            for(auto p:next_round.get_cur_next_valid_spots()){
                best_value = max(best_value, minimax(next_round, p, depth + 1, false, alpha, beta));
                alpha = max(alpha, best_value);
                if(alpha >= beta) break;
            }
            return best_value;
        } else {
            int best_value = INF_VALUE;
            for(auto p:next_round.get_cur_next_valid_spots()){
                best_value = max(best_value, minimax(next_round, p, depth + 1, true, alpha, beta));
                beta = min(beta, best_value);
                if(beta <= alpha) break;
            }
            return best_value;
        }
    }
    // return the best choice this round
    Point best_choice(){
        int max_value = -INF_VALUE;
        vector<int> value;
        int max_idx = 0;
        for(auto p : cur_round.get_cur_next_valid_spots()){
            int val = minimax(cur_round, p, 1, false, -INF_VALUE, INF_VALUE);
            value.push_back(val);
        }
        for(int i = 0; i < value.size(); i++){
            if(value[max_idx] < value[i]) max_idx = i;
        }
        Point p = cur_round.get_cur_next_valid_spots()[max_idx];
        return p;
    }
};

class Engine {
private:
    int player;
    array<array<int, SIZE>, SIZE> board;
    vector<Point> next_valid_spots;
public:
    void read_board(std::ifstream& fin) {
        fin >> player;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                fin >> board[i][j];
            }
        }
    }
    void read_valid_spots(std::ifstream& fin) {
        int n_valid_spots;
        fin >> n_valid_spots;
        int x, y;
        for (int i = 0; i < n_valid_spots; i++) {
            fin >> x >> y;
            next_valid_spots.push_back({x, y});
        }
    }

    void write_valid_spot(std::ofstream& fout) {
        int n_valid_spots = next_valid_spots.size();
        OthelloBoard first_round(board, next_valid_spots, player);
        AI ai(first_round);
        Point p = ai.best_choice();
        // Remember to flush the output to ensure the last action is written to file.
        fout << p.x << " " << p.y << std::endl;
        fout.flush();
    }
};

int main(int, char **argv)
{
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    Engine engine;
    engine.read_board(fin);
    engine.read_valid_spots(fin);
    engine.write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}