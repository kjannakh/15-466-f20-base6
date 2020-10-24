#include "Mode.hpp"

#include "Connection.hpp"
#include "ColorTextureProgram.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode(Client &client);
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----'

	void compute_checkerboard();
	void init_checkers();
	void clearSelections();
	void make_move();
	void send_data();

	enum tile_state {EMPTY, BLACK, RED, BQUEEN, RQUEEN };
	struct checkerboard_tile {
		float left;
		float bot;
		float right;
		float top;
		tile_state state;
		glm::u8vec4 color;
		int i;
		int j;
	};
	struct checkerboard_tile checkerboard[8][8];
	tile_state tempboard[8][8];
	uint8_t speedboard[8][8];

	struct checkerboard_tile from_tile;
	struct checkerboard_tile orig_tile;

	unsigned int prev_draw_x = 0;
	unsigned int prev_draw_y = 0;

	struct Selection {
		int i;
		int j;
	} hover_tile;
	std::vector< struct Selection > selected_tiles;

	glm::vec2 cursor;
	glm::vec2 click;
	glm::vec2 drawsize;

	bool terminal_move = false;
	bool skipped_move = false;

	uint8_t playerid = 0;
	uint8_t score = 0;
	uint8_t enemy_score = 0;
	uint8_t prev_game_mode = 0;
	uint8_t game_mode = 0;
	bool ready = false;
	uint8_t pieces_taken = 0;
	uint8_t player_turn = 0;

	float countdown_timer = 5.0f;
	float speed_timer = 0.0f;
	enum Speed_State { INACTIVE, COUNTDOWN, ACTIVE, WAITING };
	Speed_State speed_challenge = INACTIVE;
	uint8_t clicked = 1;

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} clear, space, probe;

#define HEX_TO_U8VEC4( HX ) (glm::u8vec4( (HX >> 24) & 0xff, (HX >> 16) & 0xff, (HX >> 8) & 0xff, (HX) & 0xff ))
	const glm::u8vec4 light_tile_color = HEX_TO_U8VEC4(0xede7c7ff);
	const glm::u8vec4 dark_tile_color = HEX_TO_U8VEC4(0x805a2aff);
	const glm::u8vec4 white_color = HEX_TO_U8VEC4(0xffffffff);
	const glm::u8vec4 yellow_tinge = HEX_TO_U8VEC4(0xfcf51e66);
	const glm::u8vec4 red_tinge = HEX_TO_U8VEC4(0xfc351e88);
#undef HEX_TO_U8VEC4

	//taken from base0 NotPong code
	struct Vertex {
		Vertex(glm::vec3 const& Position_, glm::u8vec4 const& Color_, glm::vec2 const& TexCoord_) :
			Position(Position_), Color(Color_), TexCoord(TexCoord_) { }
		glm::vec3 Position;
		glm::u8vec4 Color;
		glm::vec2 TexCoord;
	};
	static_assert(sizeof(Vertex) == 4 * 3 + 1 * 4 + 4 * 2, "Vertex should be packed");

	//Buffer used to hold vertex data during drawing:
	GLuint vertex_buffer = 0;

	//Vertex Array Object that maps buffer locations to color_texture_program attribute locations:
	GLuint vertex_buffer_for_color_texture_program = 0;

	GLuint white_tex;
	GLuint black_checker_tex;
	GLuint red_checker_tex;
	GLuint black_queen_tex;
	GLuint red_queen_tex;

	//last message from server:
	std::string server_message;

	//connection to server:
	Client &client;

};
