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
    Point now_point;
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
    OthelloBoard(array<array<int, SIZE>, SIZE> board, Point now_point, int cur_player)
    :board(board), now_point(now_point), cur_player(cur_player) {}
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
    array<array<int, SIZE>, SIZE> get_cur_board(){
        return board;
    }
    //
    int get_cur_player(){
        return cur_player;
    }
    //
    vector<Point> get_next_valid_spots(){
        return next_valid_spots;
    }
};

class AI {
private:
    // state value
    int state_value[8][8] = {
        500, -25, 10, 5, 5, 10, -25, 500,
        -25, -45, 1, 1, 1, 1, -45, -25,
        10, 1, 3, 2, 2, 3, 1, 10,
        5, 1, 2, 1, 1, 2, 1, 5,
        5, 1, 2, 1, 1, 2, 1, 5,
        10, 1, 3, 2, 2, 3, 1, 10,
        -25, -45, 1, 1, 1, 1, -45, -25,
        500, -25, 10, 5, 5, 10, -25, 500
    };
    // informations
    int player;
    int limit_depth = 4;
    array<array<int, SIZE>, SIZE> board;
    vector<Point> next_valid_spots;
public:
    AI(int player, array<array<int, SIZE>, SIZE> board, vector<Point> next_valid_spots)
    :player(player), board(board), next_valid_spots(next_valid_spots) {}
    // state value
    int evaluation(Point point){
        int spot_value = 0;
        spot_value += state_value[point.x][point.y];
        return spot_value;
    }
    // minimax recursion ()
    // cur board , next dics position on cur board , search depth , opponent or me , white or black 
    int minimax(array<array<int, SIZE>, SIZE> now_board, Point point, int depth, bool player_type, int cur_player, int alpha, int beta){
        if(depth == limit_depth){
            return evaluation(point);
        }
        OthelloBoard round(now_board, point, cur_player);
        // 若是搜尋到點無法放置時該怎麼辦 ? 
        round.put_disc(point);
        if(player_type){
            int best_value = -INF_VALUE;
            for(auto p : round.get_next_valid_spots()){
                best_value = max(best_value, minimax(round.get_cur_board(), p, depth + 1, false, round.get_cur_player(), alpha, beta));
                alpha = max(alpha, best_value);
                if(alpha >= beta) break;
            }
            return best_value;
        } else {
            int best_value = INF_VALUE;
            for(auto p : round.get_next_valid_spots()){
                best_value = min(best_value, minimax(round.get_cur_board(), p, depth + 1, true, round.get_cur_player(), alpha, beta));
                beta = min(beta, best_value);
                if(beta <= alpha) break;
            }
            return best_value;
        }
    }
    // return the best choice this round
    Point best_choice(){
        int max_value = -INF_VALUE;
        int n_valid_spots_value[50] = {0};
        int choice_idx = -1;
        for(int i = 0; i < next_valid_spots.size(); i++){
            n_valid_spots_value[i] = minimax(board, next_valid_spots[i], 1, false, player, -INF_VALUE, INF_VALUE);
            if(max_value < n_valid_spots_value[i]){
                max_value = n_valid_spots_value[i];
                choice_idx = i;
            }
        }
        return next_valid_spots[choice_idx];
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
        AI ai(player, board, next_valid_spots);
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