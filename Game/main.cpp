#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <map> 
#include <ctime>
#include <chrono>
#include <random>
#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;

struct Move {
    int i;
    int j;
    int score;
};

struct CacheNode {
    int score;
    int depth;
    int Flag;
};

map<int, int> state_cache;
map<int, CacheNode> cache;
int state_cache_puts = 0;
bool move_sorter(Move const& move1, Move const& move2) {
    return move1.score > move2.score;
}
int cache_hits = 0;
int cache_cutoffs = 0;
int cache_puts = 0;
int maximum_depth = 0;
int state_cache_hits = 0;
Move best_move;

const int max_board_size = 21;
int board_size = max_board_size;
const int win_detected = numeric_limits<int>::min() + 1;

const int live_one = 10;
const int dead_one = 1;

const int live_two = 100;
const int dead_two = 10;

const int live_three = 1000;
const int dead_three = 100;

const int live_four = 10000;
const int dead_four = 1000;

const int five = 100000;

int GameBoard[max_board_size][max_board_size];

int evaluate_block(int blocks, int pieces) {
    if (blocks == 0) {
        switch (pieces) {
			case 1:
				return live_one;
			case 2:
				return live_two;
			case 3:
				return live_three;
			case 4:
				return live_four;
			default:
				return five;
        }
    }
	
    if (blocks == 1) {
	    switch (pieces) {
			case 1:
				return dead_one;
			case 2:
				return dead_two;
			case 3:
				return dead_three;
			case 4:
				return dead_four;
			default:
				return five;
	    }
    }
	
    if (pieces >= 5) {
	    return five;
    }
	
    return 0;
}

int eval_board(int board[max_board_size][max_board_size], int piece_type, array<int, 4> const& restrictions) {
    int score = 0;
    const int min_r = restrictions[0];
    const int min_c = restrictions[1];
    const int max_r = restrictions[2];
    const int max_c = restrictions[3];
	
    for (int row = min_r; row <= max_r; row++) {
        for (int column = min_c; column <= max_c; column++) {
            if (board[row][column] == piece_type) {
                int block = 0;
                int piece = 1;
            	
                // left
                if (column == 0 || board[row][column - 1]) {
                    block++;
                }
            	
                // piece number
                for (column++; column < board_size && board[row][column] == piece_type; column++) {
                    piece++;
                }
            	
                // right
                if (column == board_size || board[row][column]) {
                    block++;
                }
            	
                score += evaluate_block(block, piece);
            }
        }
    }

    for (int column = min_c; column < max_c + 1; column++) {
        for (int row = min_r; row < max_r + 1; row++) {
            if (board[row][column] == piece_type) {
                int block = 0;
                int piece = 1;
            	
                // left
                if (row == 0 || board[row - 1][column]) {
                    block++;
                }
            	
                // piece number
                for (row++; row < board_size && board[row][column] == piece_type; row++) {
                    piece++;
                }
            	
                // right
                if (row == board_size || board[row][column]) {
                    block++;
                }
            	
                score += evaluate_block(block, piece);
            }
        }
    }

    for (int n = min_r; n < (max_c - min_c + max_r); n++) {
        int r = n;
        int c = min_c;
    	
        while (r >= min_r && c <= max_c) {
            if (r <= max_r) {
                if (board[r][c] == piece_type) {
                    int block = 0;
                    int piece = 1;
                	
                    // left
                    if (c == 0 || r == board_size - 1 || board[r + 1][c - 1]) {
                        block++;
                    }
                	
                    // piece number
                    r--;
                    c++;
                    for (; r >= 0 && board[r][c] == piece_type; r--) {
                        piece++;
                        c++;
                    }
                	
                    // right
                    if (r < 0 || c == board_size || board[r][c]) {
                        block++;
                    }
                	
                    score += evaluate_block(block, piece);
                }
            }
        	
            r--;
            c++;
        }
    }

    for (int n = min_r - (max_c - min_c); n <= max_r; n++) {
        int r = n;
        int c = min_c;
        while (r <= max_r && c <= max_c) {
            if (r >= min_r && r <= max_r) {
                if (board[r][c] == piece_type) {
                    int  block = 0;
                    int piece = 1;
                	
                    // left
                    if (c == 0 || r == 0 || board[r - 1][c - 1]) {
                        block++;
                    }
                	
                    // piece number
                    r++;
                    c++;
                    for (; r < board_size && board[r][c] == piece_type; r++) {
                        piece++;
                        c++;
                    }
                	
                    // right
                    if (r == board_size || c == board_size || board[r][c]) {
                        block++;
                    }
                	
                    score += evaluate_block(block, piece);
                }
            }
            r++;
            c++;
        }
    }
	
    return score;
}

array<array<int, 9>, 4> get_directions(int board[max_board_size][max_board_size], int x, int y) {
    array<int, 9> a,b,c,d;
    int a_i = 0, b_i = 0, c_i = 0, d_i = 0;

    for (int i = -4; i < 5; i++) {
        if (x + i >= 0 && x + i < board_size) {
            a[a_i] = board[x + i][y];
            a_i++;
        	
            if (y + i >= 0 && y + i < board_size) {
                b[b_i] = board[x + i][y + i];
                b_i++;
            }
        }
    	
        if (y + i >= 0 && y + i < board_size) {
            c[c_i] = board[x][y + i];
            c_i++;
        	
            if (x - i >= 0 && x - i < board_size) {
                d[d_i] = board[x - i][y + i];
                d_i++;
            }
        }
    }
	
    if (a_i != 9) {
        a[a_i] = 2;
    }
	
    if (b_i != 9) {
        b[b_i] = 2;
    }
	
    if (c_i != 9) {
        c[c_i] = 2;
    }
	
    if (d_i != 9) {
        d[d_i] = 2;
    }
	
    return { a,b,c,d };
}

bool check_directions(array<int, 9> const& arr) {
	const int size = 9;
	
    for (int i = 0; i < size - 4; i++) {
        if (arr[i]) {
            if (arr[i] == 2 || arr[i + 1] == 2 || arr[i + 2] == 2 || arr[i + 3] == 2 || arr[i + 4] == 2) {
                return false;
            }
        	
            if (arr[i] == arr[i + 1] && arr[i] == arr[i + 2] && arr[i] == arr[i + 3] && arr[i] == arr[i + 4]) {
                return true;
            }
        }
    }
    return false;
}

bool check_win(int board[max_board_size][max_board_size], int x, int y) {
    array<array<int, 9>, 4> directions = get_directions(board, x, y);
	
    for (int i = 0; i < 4; i++) {
        if (check_directions(directions[i])) {
            return true;
        }
    }
	
    return false;
}

bool remote_cell(int board[max_board_size][max_board_size], int r, int c) {
    for (int i = r - 2; i <= r + 2; i++) {
        if (i < 0 || i >= board_size) continue;
        for (int j = c - 2; j <= c + 2; j++) {
            if (j < 0 || j >= board_size) continue;
            if (board[i][j]) return false;
        }
    }
    return true;
}

array<int, 4> get_restrictions(int board[max_board_size][max_board_size]) {
    int min_r = numeric_limits<int>::max() - 1;
    int min_c = numeric_limits<int>::max() - 1;
    int max_r = numeric_limits<int>::min() + 1;
    int max_c = numeric_limits<int>::min() + 1;
	
    for (int i = 0; i < board_size; i++) {
        for (int j = 0; j < board_size; j++) {
            if (board[i][j]) {
                min_r = min(min_r, i);
                min_c = min(min_c, j);
                max_r = max(max_r, i);
                max_c = max(max_c, j);
            }
        }
    }
	
    if (min_r - 2 < 0) {
        min_r = 2;
    }
    if (min_c - 2 < 0) {
        min_c = 2;
    }
    if (max_r + 2 >= board_size) {
        max_r = board_size - 3;
    }
    if (max_c + 2 >= board_size) {
        max_c = board_size - 3;
    }
	
    return { min_r, min_c, max_r, max_c };
}

array<int, 4> change_restrictions(array<int, 4> const& restrictions, int i, int j) {
    int min_r = restrictions[0];
    int min_c = restrictions[1];
    int max_r = restrictions[2];
    int max_c = restrictions[3];
    if (i < min_r) {
        min_r = i;
    }
    else if (i > max_r) {
        max_r = i;
    }
    if (j < min_c) {
        min_c = j;
    }
    else if (j > max_c) {
        max_c = j;
    }
    if (min_r - 2 < 0) {
        min_r = 2;
    }
    if (min_c - 2 < 0) {
        min_c = 2;
    }
    if (max_r + 2 >= board_size) {
        max_r = board_size - 3;
    }
    if (max_c + 2 >= board_size) {
        max_c = board_size - 3;
    }

    return { min_r, min_c, max_r, max_c };
}

int get_seq(int y, int e) {
    if (y + e == 0) {
        return 0;
    }
    if (y != 0 && e == 0) {
        return y;
    }
    if (y == 0 && e != 0) {
        return -e;
    }
	// TODO why 17
    return 17;
}

int eval_pos(int seq) {
    switch (seq) {
		case 0:
			return 7;
		case 1:
			return 35;
		case 2:
			return 800;
		case 3:
			return 15000;
		case 4:
			return 800000;
		case -1:
			return 15;
		case -2:
			return 400;
		case -3:
			return 1800;
		case -4:
			return 100000;
		default: 
			return 0;
    }
}

int evaluate_state(int board[max_board_size][max_board_size], int player, int hash, array<int, 4> const& restrictions) {
    const int x_score = eval_board(board, 1, restrictions);
	const int o_score = eval_board(board, -1, restrictions);
    const int score = (x_score - o_score) * player;
    state_cache[hash] = score;
    state_cache_puts++;
	
    return score;
}

int evaluate_direction(array<int, 9> const& direction_arr, int player) {
    int score = 0;
    for (int i = 0, arr_size = direction_arr.size(); i + 4 < arr_size; i++) {
        int you = 0;
        int enemy = 0;
        if (direction_arr[i] == 2) {
            return score;
        }
        for (int j = 0; j <= 4; j++) {
            if (direction_arr[i + j] == 2) {
                return score;
            }
            if (direction_arr[i + j] == player) {
                you++;
            }
            else if (direction_arr[i + j] == -player) {
                enemy++;
            }
        }
        score += eval_pos(get_seq(you, enemy));
        if (score >= 800000) {
            return win_detected;
        }
    }
    return score;
}

int evaluate_move(int board[max_board_size][max_board_size], int x, int y, int player) {
    int score = 0;
    array<array<int, 9>, 4> directions = get_directions(board, x, y);
    for (int i = 0; i < 4; i++) {
	    const int temp_score = evaluate_direction(directions[i], player);
        if (temp_score == win_detected) {
            return win_detected;
        }
        
        score += temp_score;
    }
    return score;
}

vector<Move> board_generator(array<int, 4> const& restrictions, int board[max_board_size][max_board_size], int player) {
    vector<Move> available_spots_score;
    const int  min_r = restrictions[0];
    const int min_c = restrictions[1];
    const int max_r = restrictions[2];
    const int max_c = restrictions[3];
    for (int i = min_r - 2; i <= max_r + 2; i++) {
        for (int j = min_c - 2; j <= max_c + 2; j++) {
            if (board[i][j] == 0 && !remote_cell(board, i, j)) {
                Move move;
                move.i = i;
                move.j = j;
                move.score = evaluate_move(board, i, j, player);
                if (move.score == win_detected) {
                    vector<Move> winning_move = { move };
                    return winning_move;
                }
                available_spots_score.push_back(move);
            }
        }
    }
    sort(available_spots_score.begin(), available_spots_score.end(), move_sorter);
	
    return available_spots_score;
}

int Table[max_board_size][max_board_size][2];
mt19937 mt_rand(time(nullptr));
void table_init() {
    for (int i = 0; i < board_size; i++) {
        for (int j = 0; j < board_size; j++) {
            Table[i][j][0] = mt_rand(); //1
            Table[i][j][1] = mt_rand(); //2
        }
    }
}

int hash_board(int board[max_board_size][max_board_size]) {
    int hash = 0;
	
    for (int i = 0; i < board_size; i++) {
        for (int j = 0; j < board_size; j++) {
            if (board[i][j]) {
                hash ^= Table[i][j][(board[i][j]+1)/2];
            }
        }
    }
	
    return hash;
}

int  update_hash(int hash, int player, int row, int col) {
    hash ^= Table[row][col][(player + 1) / 2];
    return hash;
}

int nega_max(int newBoard[max_board_size][max_board_size], int player, int depth, int a, int b, int hash, array<int, 4> const& restrictions, int last_i, int last_j) {
    const int alphaOrig = a;
	
    if (cache.count(hash) && cache[hash].depth >= depth) {
        cache_hits++;
        int score = cache[hash].score;
        if (cache[hash].Flag == 0) {
            cache_cutoffs++;
            return score;
        }
        if (cache[hash].Flag == -1) {
            a = max(a, score);
        }
        else if (cache[hash].Flag == 1) {
            b = min(b, score);
        }
        if (a >= b) {
            cache_cutoffs++;
            return score;
        }
    }

    if (check_win(newBoard, last_i, last_j)) {
        return -2000000 + (maximum_depth - depth);
    }
    if (depth == 0) {
        if (state_cache.count(hash)) {
            state_cache_hits++;
            return state_cache[hash];
        }
    	
        return evaluate_state(newBoard, player, hash, restrictions);
    }
    const vector<Move> availableSpots = board_generator(restrictions, newBoard, player);

    if (availableSpots.empty()) {
        return 0;
    }

    int best_value = numeric_limits<int>::min() + 1;
    for (const Move& availableSpot : availableSpots)
    {
        const int i = availableSpot.i;
        const int j = availableSpot.j;

        const int newHash = update_hash(hash, player, i, j);
        newBoard[i][j] = player;
        int value = -nega_max(newBoard, -player, depth - 1, -b, -a, newHash, change_restrictions(restrictions, i, j), i, j);
        newBoard[i][j] = 0;
        if (value > best_value) {
            best_value = value;
            if (depth == maximum_depth) {
                best_move = { i,j,value };
				cout << "best move" << endl;
                cout << best_move.i << " " << best_move.j << " " << best_move.score << endl;
            }
        }
        a = max(a, value);
        if (a >= b) {
            break;
        }
    }
	
    cache_puts++;
    CacheNode cache_node;

    cache_node.score = best_value;
    cache_node.depth = depth;
    if (best_value <= alphaOrig) {
        cache_node.Flag = 1;
    }
    else if (best_value >= b) {
        cache_node.Flag = -1;
    }
    else {
        cache_node.Flag = 0;
    }
    cache[hash] = cache_node;
    return best_value;
}

Move mtd_f(int board[max_board_size][max_board_size], int player, int guess, int depth, array<int, 4> const& restrictions) {
    int upper_bound = numeric_limits<int>::max() - 1;
    int lower_bound = numeric_limits<int>::min() + 1;
    Move last_successful;
	
    do {
	    const int guess_lb = guess + (lower_bound == guess);
        guess = nega_max(board, player, depth, guess_lb - 1, guess_lb, hash_board(board), restrictions, 0, 0);
        last_successful = best_move;
        guess < guess_lb ? upper_bound = guess : lower_bound = guess;
    } while (lower_bound < upper_bound);

	return last_successful;
}

Move iterative_mtd_f(const int depth, const int player) {
    const array<int, 4> restrictions = get_restrictions(GameBoard);
    int guess = evaluate_state(GameBoard, player, hash_board(GameBoard), restrictions);

	for (int i = 2; i != depth + 2; i += 2)
    {
        maximum_depth = i;
        guess = mtd_f(GameBoard, player, guess, i, get_restrictions(GameBoard)).score;
        if (guess > 1999900) break;
    }
	
    return best_move;
}

void init_GameBoard(int player)
{
	for (int i = 0; i < board_size; i++)
	{
		for (int j = 0; j < board_size; j++)
		{
            GameBoard[i][j] = 0;
		}
	}
	
	if (player == -1)
	{
        GameBoard[board_size / 2][board_size / 2] = 1;
	}
}

int main()
{
    maximum_depth = 8;
    int depth = 2;
    int show = 0;
	
    const int move_x = 200;
    const int move_y = 25;
    const int cell = 43;
	
    int player = -1;
    init_GameBoard(player);
    table_init();

    int new_depth = depth;
    int new_size = board_size;
    bool result = false;
    int last_x = -1;
    int last_y = -1;
	
    RenderWindow window(VideoMode(400 + 43 * board_size, 50 + 43 * board_size), "Gomoku", Style::Titlebar | Style::Close);
    window.setPosition(Vector2i(0, 0));

    Texture t;
    t.loadFromFile("square.png");
    Sprite s(t);
	
    Texture c1;
    c1.loadFromFile("circle.png");
    c1.setSmooth(true);
    Sprite circle(c1);

    Texture c2;
    c2.loadFromFile("cross.png");
    c2.setSmooth(true);
    Sprite cross(c2);

    Font font;
    font.loadFromFile("Corben-Bold.ttf");
    Text text("Gomoku", font, 26);
    text.setFillColor(Color::Black);
    text.setPosition(25, 25);

    Texture gx;
    gx.loadFromFile("game_x.png");
    Sprite game_x(gx);
    game_x.setPosition(35, 65);

    Texture go;
    go.loadFromFile("game_o.png");
    Sprite game_o(go);
    game_o.setPosition(35, 165);

    Text result_win("WIN!", font, 28);
    result_win.setFillColor(Color::Green);
    result_win.setOutlineThickness(2.5f);
    result_win.setOutlineColor(Color::Black);
    result_win.setPosition(48, 280);

    Text result_defeat("DEFEAT!", font, 28);
    result_defeat.setFillColor(Color::Red);
    result_defeat.setOutlineThickness(2.5f);
    result_defeat.setOutlineColor(Color::Black);
    result_defeat.setPosition(25, 280);

    Text error("7 <= Size <= 21", font, 20);
    error.setFillColor(Color::Red);
    error.setOutlineThickness(2.5f);
    error.setOutlineColor(Color::Black);
    error.setPosition(25, 280);
	
    Text levels("Levels", font, 24);
    levels.setFillColor(Color::Black);
    levels.setPosition(48 + board_size*cell + 200, 25);
	
    Texture e;
    e.loadFromFile("easy.png");
    Sprite easy(e);
    easy.setPosition(35 + board_size * cell + 200,60);

    Texture n;
    n.loadFromFile("normal.png");
    Sprite normal(n);
    normal.setPosition(35 + board_size * cell + 200, 110);

    Texture h;
    h.loadFromFile("hard.png");
    Sprite hard(h);
    hard.setPosition(35 + board_size * cell + 200, 160);

    Text level_text("easy", font, 22);
    level_text.setOrigin(0, 0);
    level_text.setFillColor(Color::Black);
    level_text.setPosition(60 + board_size * cell + 200, 210);
	
    Texture sz;
    sz.loadFromFile("size.png");
    Sprite size(sz);
    size.setPosition(35 + board_size * cell + 200, 250);
	
    String size_input = "21";
    Text size_text(size_input, font, 24);
    size_text.setFillColor(Color::Black);
    size_text.setPosition(80 + board_size * cell + 200, 300);

    RectangleShape last_x_shape(Vector2f(50, 50));
    last_x_shape.setFillColor(Color(255, 0, 0, 150));
	
    RectangleShape last_o_shape(Vector2f(50, 50));
    last_o_shape.setFillColor(Color(255, 0, 0, 150));
	
    while (window.isOpen())
    {
        Vector2i pos = Mouse::getPosition(window);
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                window.close();
            }
        	if (event.key.code == Keyboard::Backspace && size_input.getSize() > 0)
        	{
                size_input = "";
                size_text.setString(size_input);
        	}
            else if (event.type == Event::TextEntered && size_input.getSize() < 2)
            {
                String s = event.text.unicode;
                char code = s.toAnsiString()[0];
            	if (code >= 48 && code < 58)
            	{
                    size_input += event.text.unicode;
                    size_text.setString(size_input);
            	}
            }
            if (event.type == Event::MouseButtonPressed && event.key.code == Mouse::Left)
            {
            	
            	if (pos.x >= game_x.getPosition().x && pos.y >= game_x.getPosition().y
                    && pos.x < game_x.getPosition().x + gx.getSize().x && pos.y < game_x.getPosition().y + gx.getSize().y)
            	{
                    show = 0;
                    player = 1;
                    board_size = new_size;
                    result = false;
                    size_text.setPosition(80 + board_size * cell + 200, 300);
                    size.setPosition(35 + board_size * cell + 200, 250);
                    level_text.setPosition(60 + board_size * cell + 200, 210);
                    hard.setPosition(35 + board_size * cell + 200, 160);
                    normal.setPosition(35 + board_size * cell + 200, 110);
                    easy.setPosition(35 + board_size * cell + 200, 60);
                    levels.setPosition(48 + board_size * cell + 200, 25);
                    last_x_shape.setPosition(Vector2f(0, 0));
                    last_o_shape.setPosition(Vector2f(0, 0));
            		
                    window.setSize(Vector2u(400 + 43 * board_size, 50 + 43 * board_size));
                    View view(Vector2f(window.getSize().x/2.0, window.getSize().y / 2.0), 
                        Vector2f(window.getSize().x, window.getSize().y));
					window.setView(view);
            		
                    depth = new_depth;
                    cache.clear();
                    state_cache.clear();
                    init_GameBoard(player);
                    table_init();
            	}
                else if (pos.x >= game_o.getPosition().x && pos.y >= game_o.getPosition().y
                    && pos.x < game_o.getPosition().x + go.getSize().x && pos.y < game_o.getPosition().y + go.getSize().y)
                {
                    show = 0;
                    player = -1;
                    board_size = new_size;
                    result = false;
                    size_text.setPosition(80 + board_size * cell + 200, 300);
                    size.setPosition(35 + board_size * cell + 200, 250);
                    level_text.setPosition(60 + board_size * cell + 200, 210);
                    hard.setPosition(35 + board_size * cell + 200, 160);
                    normal.setPosition(35 + board_size * cell + 200, 110);
                    easy.setPosition(35 + board_size * cell + 200, 60);
                    levels.setPosition(48 + board_size * cell + 200, 25);
                    window.setSize(Vector2u(400 + 43 * board_size, 50 + 43 * board_size));
                    View view(Vector2f(window.getSize().x / 2.0, window.getSize().y / 2.0),
                        Vector2f(window.getSize().x, window.getSize().y));
                    window.setView(view);
                    last_x_shape.setPosition(Vector2f(0, 0));
                    last_o_shape.setPosition(Vector2f(0, 0));
                	
                    depth = new_depth;
                    cache.clear();
                    state_cache.clear();
                    init_GameBoard(player);
                    table_init();
                }
                else if (pos.x >= easy.getPosition().x && pos.y >= easy.getPosition().y
                    && pos.x < easy.getPosition().x + e.getSize().x && pos.y < easy.getPosition().y + e.getSize().y)
                {
                    new_depth = 2;
                    level_text.setString("easy");
                }
                else if (pos.x >= normal.getPosition().x && pos.y >= normal.getPosition().y
                    && pos.x < normal.getPosition().x + n.getSize().x && pos.y < normal.getPosition().y + n.getSize().y)
                {
                    new_depth = 4;
                    level_text.setString("norm");
                }
                else if (pos.x >= hard.getPosition().x && pos.y >= hard.getPosition().y
                    && pos.x < hard.getPosition().x + h.getSize().x && pos.y < hard.getPosition().y + h.getSize().y)
                {
                    new_depth = 6;
                    level_text.setString("hard");
                }
                else if (pos.x >= size.getPosition().x && pos.y >= size.getPosition().y
                    && pos.x < size.getPosition().x + sz.getSize().x && pos.y < size.getPosition().y + sz.getSize().y)
                {
                	int temp = stoi(size_input.toAnsiString());
                	if (temp < 7 || temp > 21)
                	{
                        show = 3;
                	}
                    else
                    {
                        show = show == 3 ? 0 : show;
                        new_size = temp;
                    }
                }
	            else if (!result && pos.x >= move_x + 7 && pos.y >= move_y+7
                    && pos.x < board_size*43 + move_x + 7 && pos.y < board_size*43 + move_y + 7)
	            {
                    last_x = (pos.y - (move_y + 7)) / 43;
                    last_y = (pos.x - (move_x + 7)) / 43;
	            	if (!GameBoard[last_x][last_y])
	            	{
                        GameBoard[last_x][last_y] = player;
	            		
	            		if (player == 1)
	            		{
                            last_x_shape.setPosition(last_y * cell + move_x, last_x * cell + move_y);
	            		}
                        else if (player == -1)
                        {
                            last_o_shape.setPosition(last_y * cell + move_x, last_x * cell + move_y);
                        }
	            		
	            		if (check_win(GameBoard, last_x, last_y))
	            		{
                            show = 1;
                            result = true;
	            		}
	            	}
                    else
                    {
                        last_x = last_y = -1;
                    }
	            }
            }
        }
        window.clear(Color(245, 245, 220));
        window.draw(text);
        window.draw(easy);
        window.draw(normal);
        window.draw(hard);
        window.draw(game_x);
        window.draw(game_o);
        window.draw(size);
        window.draw(levels);
        window.draw(size_text);
        window.draw(level_text);
    	if (last_x_shape.getPosition().x > 0)
    	{
            window.draw(last_x_shape);
    	}
        if (last_o_shape.getPosition().x > 0)
        {
            window.draw(last_o_shape);
        }
    	if (show == 1)
    	{
            window.draw(result_win);
    	}
        else if (show == 2)
        {
            window.draw(result_defeat);
        }
        else if (show == 3)
        {
            window.draw(error);
        }
    	
        for (int i = 0; i < board_size; i++)
        {
	        for (int j = 0; j < board_size; j++)
	        {
                s.setPosition(j * cell + move_x, i * cell + move_y);
                window.draw(s);
	        	if(GameBoard[i][j] == 1)
	        	{
                    cross.setPosition(j * cell + move_x + 7, i * cell + move_y + 7);
                    window.draw(cross);
	        	}
                else if (GameBoard[i][j] == -1)
                {
                    circle.setPosition(j * cell + move_x + 7, i * cell + move_y + 7);
                    window.draw(circle);
                }
	        }
        }
        window.display();
    	
        if (last_x != -1 && last_y != -1)
        {
            best_move = iterative_mtd_f(depth, player);
            GameBoard[best_move.i][best_move.j] = -player;
            if (player == -1)
            {
                last_x_shape.setPosition(best_move.j * cell + move_x, best_move.i * cell + move_y);
            }
            else if (player == 1)
            {
                last_o_shape.setPosition(best_move.j * cell + move_x, best_move.i * cell + move_y);
            }
            if (check_win(GameBoard, best_move.i, best_move.j))
            {
                show = 2;
                result = true;
            }
            last_x = -1;
            last_y = -1;
        }
    }
	
    cache.clear();
    state_cache.clear();
	
    return 0;
}