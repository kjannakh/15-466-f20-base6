#include "PlayMode.hpp"

#include "DrawLines.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "hex_dump.hpp"
#include "ColorTextureProgram.hpp"
#include "load_save_png.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

PlayMode::PlayMode(Client &client_) : client(client_) {
	//----- allocate OpenGL resources -----
	{ //vertex buffer:
		glGenBuffers(1, &vertex_buffer);
		//for now, buffer will be un-filled.

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	{ //vertex array mapping buffer for color_texture_program:
		//ask OpenGL to fill vertex_buffer_for_color_texture_program with the name of an unused vertex array object:
		glGenVertexArrays(1, &vertex_buffer_for_color_texture_program);

		//set vertex_buffer_for_color_texture_program as the current vertex array object:
		glBindVertexArray(vertex_buffer_for_color_texture_program);

		//set vertex_buffer as the source of glVertexAttribPointer() commands:
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

		//glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(color_texture_program->Position_vec4);

		//set up the vertex array object to describe arrays of Vertex:
		glVertexAttribPointer(
			color_texture_program->Position_vec4, //attribute
			3, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte*)0 //offset
		);
		//[Note that it is okay to bind a vec3 input to a vec4 attribute -- the w component will be filled with 1.0 automatically]
		glEnableVertexAttribArray(color_texture_program->Color_vec4);
		glVertexAttribPointer(
			color_texture_program->Color_vec4, //attribute
			4, //size
			GL_UNSIGNED_BYTE, //type
			GL_TRUE, //normalized
			sizeof(Vertex), //stride
			(GLbyte*)0 + 4 * 3 //offset
		);

		glEnableVertexAttribArray(color_texture_program->TexCoord_vec2);
		glVertexAttribPointer(
			color_texture_program->TexCoord_vec2, //attribute
			2, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Vertex), //stride
			(GLbyte*)0 + 4 * 3 + 4 * 1 //offset
		);

		//done referring to vertex_buffer, so unbind it:
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//done setting up vertex array object, so unbind it:
		glBindVertexArray(0);
	}
	{ //solid white texture:
		//ask OpenGL to fill white_tex with the name of an unused texture object:
		glGenTextures(1, &white_tex);

		//bind that texture object as a GL_TEXTURE_2D-type texture:
		glBindTexture(GL_TEXTURE_2D, white_tex);

		//upload a 1x1 image of solid white to the texture:
		glm::uvec2 size = glm::uvec2(1, 1);
		std::vector< glm::u8vec4 > data(size.x * size.y, glm::u8vec4(0xff, 0xff, 0xff, 0xff));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

		//set filtering and wrapping parameters:
		//(it's a bit silly to mipmap a 1x1 texture, but I'm doing it because you may want to use this code to load different sizes of texture)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//since texture uses a mipmap and we haven't uploaded one, instruct opengl to make one for us:
		glGenerateMipmap(GL_TEXTURE_2D);

		//Okay, texture uploaded, can unbind it:
		glBindTexture(GL_TEXTURE_2D, 0);

		GL_ERRORS(); //PARANOIA: print out any OpenGL errors that may have happened
	}

	std::vector< glm::u8vec4 > data;
	glm::uvec2 size(0, 0);
	load_png(data_path("../assets/black_checker.png"), &size, &data, LowerLeftOrigin);

	glGenTextures(1, &black_checker_tex);
	glBindTexture(GL_TEXTURE_2D, black_checker_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data.clear();
	load_png(data_path("../assets/red_checker.png"), &size, &data, LowerLeftOrigin);

	glGenTextures(1, &red_checker_tex);
	glBindTexture(GL_TEXTURE_2D, red_checker_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data.clear();
	load_png(data_path("../assets/black_queen.png"), &size, &data, LowerLeftOrigin);

	glGenTextures(1, &black_queen_tex);
	glBindTexture(GL_TEXTURE_2D, black_queen_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data.clear();
	load_png(data_path("../assets/red_queen.png"), &size, &data, LowerLeftOrigin);

	glGenTextures(1, &red_queen_tex);
	glBindTexture(GL_TEXTURE_2D, red_queen_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	init_checkers();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.repeat) {
			//ignore repeats
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_c) {
			clear.downs += 1;
			clear.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_p) {
			probe.downs += 1;
			probe.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_c) {
			space.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_p) {
			probe.pressed = false;
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			click.x = float(evt.motion.x);
			click.y = drawsize.y - float(evt.motion.y);
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEBUTTONUP) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEMOTION) {
		cursor.x = float(evt.motion.x);
		cursor.y = drawsize.y - float(evt.motion.y);
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//queue data for sending to server:
	//TODO: send something that makes sense for your game

	//reset button press counters:
	if (game_mode == 0) {
		if (space.downs > 0) ready = !ready;
		send_data();
	}
	else if (game_mode == 1) {
		if (space.downs > 0) {
			make_move();
		}
		if (clear.downs > 0) {
			clearSelections();
		}
		if (probe.downs > 0) {
			struct Selection selected = selected_tiles.front();
			struct checkerboard_tile tile = checkerboard[selected.i][selected.j];
			std::printf("(%d,%d) %d\n", tile.i, tile.j, tile.state);
		}
	}
	else if (game_mode == 2) {
		if (speed_challenge == COUNTDOWN) {
			countdown_timer -= elapsed;
			if (countdown_timer < 0.0f) {
				speed_challenge = ACTIVE;
				speed_timer = 0.0f;
				clearSelections();
			}
		}
		else if (speed_challenge == ACTIVE) {
			speed_timer += elapsed;
			if (space.downs > 0) {
				for (auto selected_tile : selected_tiles) {
					uint8_t idx = speedboard[selected_tile.i][selected_tile.j];
					if (idx == clicked) {
						clicked += 1;
					}
					else {
						break;
					}
				}
				if (playerid == player_turn) {
					if (clicked == 5) {
						speed_challenge = WAITING;
						send_data();
					}
				}
				else {
					if (clicked == 6) {
						speed_challenge = WAITING;
						send_data();
					}
				}
				clicked = 1;
				clearSelections();
			}
			if (speed_timer > 30.0f) {
				speed_challenge = WAITING;
				speed_timer = 35.0f;
				send_data();
				clicked = 1;
				clearSelections();
			}
		}
	}
	
	space.downs = 0;
	clear.downs = 0;
	probe.downs = 0;

	//send/receive data:
	client.poll([this](Connection *c, Connection::Event event){
		if (event == Connection::OnOpen) {
			std::cout << "[" << c->socket << "] opened" << std::endl;
		} else if (event == Connection::OnClose) {
			std::cout << "[" << c->socket << "] closed (!)" << std::endl;
			throw std::runtime_error("Lost connection to server!");
		} else { assert(event == Connection::OnRecv);
			//std::cout << "[" << c->socket << "] recv'd data. Current buffer:\n" << hex_dump(c->recv_buffer); std::cout.flush();
			//expecting message(s) like 'm' + 3-byte length + length bytes of text:
			while (c->recv_buffer.size() >= 70) {
				//std::cout << "[" << c->socket << "] recv'd data. Current buffer:\n" << hex_dump(c->recv_buffer); std::cout.flush();
				char type = c->recv_buffer[0];
				if (type != 'm') {
					throw std::runtime_error("Server sent unknown message type '" + std::to_string(type) + "'");
				}
				/*uint32_t size = (
					(uint32_t(c->recv_buffer[1]) << 16) | (uint32_t(c->recv_buffer[2]) << 8) | (uint32_t(c->recv_buffer[3]))
				);*/
				if (c->recv_buffer.size() < 70) break; //if whole message isn't here, can't process
				//whole message *is* here, so set current server message:
				//server_message = std::string(c->recv_buffer.begin() + 4, c->recv_buffer.begin() + 4 + size);
				if (playerid == 0)
					playerid = c->recv_buffer[1];
				else if (playerid != c->recv_buffer[1])
					throw std::runtime_error("Received message meant for another client.");

				game_mode = c->recv_buffer[2];
				
				if (prev_game_mode == 0 && game_mode == 1) {
					init_checkers();
				}
				if (prev_game_mode == 1 && game_mode == 0) {
					ready = false;
				}

				player_turn = c->recv_buffer[3];

				if (playerid == 1) {
					score = c->recv_buffer[4];
					enemy_score = c->recv_buffer[5];
				}
				else if (playerid == 2) {
					score = c->recv_buffer[5];
					enemy_score = c->recv_buffer[4];
				}
				else {
					score = c->recv_buffer[4];
					enemy_score = c->recv_buffer[5];
				}

				if (game_mode == 1) {
					for (int i = 0; i < 8; i++) {
						for (int j = 0; j < 8; j++) {
							checkerboard[i][j].state = (enum tile_state)c->recv_buffer[8 * i + j + 6];
						}
					}
				}
				else if (game_mode == 2) {
					if (prev_game_mode != 2) {
						countdown_timer = 5.0f;
						speed_timer = 0.0f;
						speed_challenge = COUNTDOWN;
						for (int i = 0; i < 8; i++) {
							for (int j = 0; j < 8; j++) {
								speedboard[i][j] = c->recv_buffer[8 * i + j + 6];
							}
						}
					}
				}

				prev_game_mode = game_mode;
				//and consume this part of the buffer:
				//c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin() + 4 + size);
				c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin() + 70);
			}
		}
	}, 0.0);
}

void PlayMode::send_data() {
	if (game_mode == 0) {
		client.connections.back().send('b');
		client.connections.back().send(game_mode);
		client.connections.back().send(playerid);
		if (ready)
			client.connections.back().send(uint8_t(1));
		else
			client.connections.back().send(uint8_t(0));
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				client.connections.back().send(uint8_t(tempboard[i][j]));
			}
		}
	}
	else if (game_mode == 1) {
		client.connections.back().send('b');
		client.connections.back().send(game_mode);
		client.connections.back().send(playerid);
		client.connections.back().send(pieces_taken);
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				client.connections.back().send(uint8_t(tempboard[i][j]));
			}
		}
	}
	else if (game_mode == 2) {
		uint32_t time = (uint32_t)(100.0f * speed_timer);
		client.connections.back().send('b');
		client.connections.back().send(game_mode);
		client.connections.back().send(playerid);
		client.connections.back().send(pieces_taken);
		client.connections.back().send(uint8_t(time >> 24));
		client.connections.back().send(uint8_t((time >> 16) % 256));
		client.connections.back().send(uint8_t((time >> 8) % 256));
		client.connections.back().send(uint8_t(time % 256));
		for (int i = 0; i < 60; i++) {
			client.connections.back().send(uint8_t(0));
		}
	}
}

void PlayMode::compute_checkerboard() {
	float tilesize = drawsize.y / 10.0f;
	float left = (drawsize.x + tilesize) / 2.0f - 4.0f * tilesize;
	float bot = tilesize;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			checkerboard[i][j].left = left + i * tilesize;
			checkerboard[i][j].right = left + (i + 1) * tilesize;
			checkerboard[i][j].bot = bot + j * tilesize;
			checkerboard[i][j].top = bot + (j + 1) * tilesize;
			if ((i + j) % 2 == 0) {
				checkerboard[i][j].color = light_tile_color;
			}
			else {
				checkerboard[i][j].color = dark_tile_color;
			}
		}
	}
}

void PlayMode::init_checkers() {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if ((i + j) % 2 == 0) {
				if (j < 3) {
					checkerboard[i][j].state = BLACK;
					tempboard[i][j] = BLACK;
				}
				else if (j > 4) {
					checkerboard[i][j].state = RED;
					tempboard[i][j] = RED;
				}
				else {
					checkerboard[i][j].state = EMPTY;
					tempboard[i][j] = EMPTY;
				}
			}
			else {
				checkerboard[i][j].state = EMPTY;
				tempboard[i][j] = EMPTY;
			}
			checkerboard[i][j].i = i;
			checkerboard[i][j].j = j;
		}
	}
}

void PlayMode::make_move() {
	int i = 0;
	if (selected_tiles.size() < 2) {
		std::printf("Not enough tiles selected\n");
		clearSelections();
		return;
	}
	for (auto selected_tile : selected_tiles) {
		if (terminal_move == true) {
			std::printf("Terminated\n");
			clearSelections();
			return;
		}
		struct checkerboard_tile tile;
		if (playerid == 1) tile = checkerboard[selected_tile.i][selected_tile.j];
		else if (playerid == 2) tile = checkerboard[7 - selected_tile.i][7 - selected_tile.j];
		if (i == 0) {
			if (playerid == 1) {
				if (tile.state != BLACK && tile.state != BQUEEN) {
					std::printf("Not black\n");
					clearSelections();
					return;
				}
			}
			else {
				if (tile.state != RED && tile.state != RQUEEN) {
					std::printf("Not red\n");
					clearSelections();
					return;
				}
			}
			orig_tile = tile;
		}
		else {
			if (tile.state != EMPTY || (tile.i + tile.j) % 2 != 0) {
				std::printf("Not empty\n");
				clearSelections();
				return;
			}
			if (from_tile.state == EMPTY && !skipped_move) {
				clearSelections();
				return;
			}
			if (orig_tile.state == BLACK) {
				if (tile.j == from_tile.j + 1) {
					if (skipped_move) {
						std::printf("One tile move on skip move\n");
						clearSelections();
						return;
					}
					if (tile.i == from_tile.i + 1 || tile.i == from_tile.i - 1) {
						terminal_move = true;
					}
					else {
						std::printf("i,j mismatch\n");
						clearSelections();
						return;
					}
				}
				else if (tile.j == from_tile.j + 2) {
					skipped_move = true;
					if (tile.i == from_tile.i + 2) {
						enum tile_state skip_state = checkerboard[from_tile.i + 1][from_tile.j + 1].state;
						if (skip_state == EMPTY || skip_state == BLACK || skip_state == BQUEEN) {
							clearSelections();
							return;
						}
					}
					else if (tile.i == from_tile.i - 2) {
						enum tile_state skip_state = checkerboard[from_tile.i - 1][from_tile.j + 1].state;
						if (skip_state == EMPTY || skip_state == BLACK || skip_state == BQUEEN) {
							clearSelections();
							return;
						}
					}
					else {
						clearSelections();
						return;
					}
				}
				else {
					clearSelections();
					return;
				}
				if (tile.j == 7) terminal_move = true;
			}
			else if (orig_tile.state == RED) {
				if (tile.j == from_tile.j - 1) {
					if (skipped_move) {
						clearSelections();
						return;
					}
					if (tile.i == from_tile.i + 1 || tile.i == from_tile.i - 1) {
						terminal_move = true;
					}
					else {
						clearSelections();
						return;
					}
				}
				else if (tile.j == from_tile.j - 2) {
					skipped_move = true;
					if (tile.i == from_tile.i + 2) {
						enum tile_state skip_state = checkerboard[from_tile.i + 1][from_tile.j - 1].state;
						if (skip_state == EMPTY || skip_state == RED || skip_state == RQUEEN) {
							clearSelections();
							return;
						}
					}
					else if (tile.i == from_tile.i - 2) {
						enum tile_state skip_state = checkerboard[from_tile.i - 1][from_tile.j - 1].state;
						if (skip_state == EMPTY || skip_state == RED || skip_state == RQUEEN) {
							clearSelections();
							return;
						}
					}
					else {
						clearSelections();
						return;
					}
				}
				else {
					clearSelections();
					return;
				}
				if (tile.j == 0) terminal_move = true;
			}
			else if (orig_tile.state == BQUEEN) {
				if (tile.j == from_tile.j + 1 || tile.j == from_tile.j - 1) {
					if (tile.i == from_tile.i + 1 || tile.i == from_tile.i - 1) {
						terminal_move = true;
						if (skipped_move) {
							clearSelections();
							return;
						}
					}
					else {
						clearSelections();
						return;
					}
				}
				else if ((tile.i == from_tile.i + 2 && tile.j == from_tile.j + 2)
					|| (tile.i == from_tile.i - 2 && tile.j == from_tile.j + 2)
					|| (tile.i == from_tile.i + 2 && tile.j == from_tile.j - 2)
					|| (tile.i == from_tile.i - 2 && tile.j == from_tile.j - 2)) {
					skipped_move = true;
					enum tile_state skip_state = checkerboard[(from_tile.i + tile.i) / 2][(from_tile.j + tile.j) / 2].state;
					if (skip_state == EMPTY || skip_state == BLACK || skip_state == BQUEEN) {
						clearSelections();
						return;
					}
				}
				else {
					clearSelections();
					return;
				}
			}
			else if (orig_tile.state == RQUEEN) {
			if (tile.j == from_tile.j + 1 || tile.j == from_tile.j - 1) {
				if (tile.i == from_tile.i + 1 || tile.i == from_tile.i - 1) {
					terminal_move = true;
					if (skipped_move) {
						clearSelections();
						return;
					}
				}
				else {
					clearSelections();
					return;
				}
			}
			else if ((tile.i == from_tile.i + 2 && tile.j == from_tile.j + 2)
				|| (tile.i == from_tile.i - 2 && tile.j == from_tile.j + 2)
				|| (tile.i == from_tile.i + 2 && tile.j == from_tile.j - 2)
				|| (tile.i == from_tile.i - 2 && tile.j == from_tile.j - 2)) {
				skipped_move = true;
				enum tile_state skip_state = checkerboard[(from_tile.i + tile.i) / 2][(from_tile.j + tile.j) / 2].state;
				if (skip_state == EMPTY || skip_state == RED || skip_state == RQUEEN) {
					clearSelections();
					return;
				}
			}
			else {
				clearSelections();
				return;
			}
			}
			else {
				clearSelections();
				return;
			}
		}
		i++;
		from_tile = tile;
	}

	for (i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			tempboard[i][j] = checkerboard[i][j].state;
		}
	}
	i = 0;
	tile_state piece = EMPTY;
	pieces_taken = 0;
	for (auto selected_tile : selected_tiles) {
		struct checkerboard_tile tile;
		if (playerid == 1) tile = checkerboard[selected_tile.i][selected_tile.j];
		else if (playerid == 2) tile = checkerboard[7 - selected_tile.i][7 - selected_tile.j];
		if (i == 0) {
			piece = checkerboard[tile.i][tile.j].state;
		}
		else {
			tempboard[from_tile.i][from_tile.j] = EMPTY;
			tempboard[tile.i][tile.j] = piece;
			if (skipped_move) {
				int skipped_i = (from_tile.i + tile.i) / 2;
				int skipped_j = (from_tile.j + tile.j) / 2;
				tempboard[skipped_i][skipped_j] = EMPTY;
				pieces_taken++;
			}
			if (playerid == 1 && tile.j == 7 && piece == BLACK) {
				tempboard[tile.i][tile.j] = BQUEEN;
			}
			else if (playerid == 2 && tile.j == 0 && piece == RED) {
				tempboard[tile.i][tile.j] = RQUEEN;
			}
		}
		i++;
		from_tile = tile;
	}
	clearSelections();

	send_data();
}

void PlayMode::clearSelections() {
	selected_tiles.clear();
	click.x = 0.0f;
	click.y = 0.0f;
	terminal_move = false;
	skipped_move = false;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	drawsize.x = float(drawable_size.x);
	drawsize.y = float(drawable_size.y);

	glDisable(GL_DEPTH_TEST);

	glm::mat4 projection = glm::ortho(0.0f, float(drawable_size.x), 0.0f, float(drawable_size.y));

	{
		glDisable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ZERO);
		DrawLines lines(projection);

		auto draw_text = [&](glm::vec2 const& at, std::string const& text, float H) {
			lines.draw_text(text,
				glm::vec3(at.x, at.y, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0x00, 0x00, 0x00, 0x00));
			float ofs = 2.0f / drawable_size.y;
			lines.draw_text(text,
				glm::vec3(at.x + ofs, at.y + ofs, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0xff, 0xff, 0xff, 0x00));
		};

		if (playerid == 1) {
			draw_text(glm::vec2(drawsize.x / 2.0f - 64.0f, drawsize.y - 40.0f), "Player 1: Black checkers", 32.0f);
		}
		else if (playerid == 2) {
			draw_text(glm::vec2(drawsize.x / 2.0f - 64.0f, drawsize.y - 40.0f), "Player 2: Red checkers", 32.0f);
		}
		else {
			draw_text(glm::vec2(drawsize.x / 2.0f - 64.0f, drawsize.y - 40.0f), "Spectating", 32.0f);
		}
		

		if (game_mode == 0) {
			if (!ready) {
				draw_text(glm::vec2(drawsize.x / 2.0f - 64.0f, drawsize.y / 2.0f), "Press space to ready up", 36.0f);
			}
			else {
				draw_text(glm::vec2(drawsize.x / 2.0f - 128.0f, drawsize.y / 2.0f), "Ready, waiting on other player...", 36.0f);
			}
			
		}
		else if (game_mode == 1) {
			draw_text(glm::vec2(checkerboard[0][0].left / 2.0f, checkerboard[0][0].bot), std::to_string(score), 32.0f);
			draw_text(glm::vec2(checkerboard[0][0].left / 2.0f, checkerboard[0][7].bot), std::to_string(enemy_score), 32.0f);
			if (playerid == player_turn) {
				draw_text(glm::vec2(drawsize.x / 2.0f - 64.0f, 20.0f), "Your turn", 32.0f);
			}
			else if (playerid < 3) {
				draw_text(glm::vec2(drawsize.x / 2.0f - 72.0f, 20.0f), "Opponent's turn", 32.0f);
			}
		}
	}

	if (drawable_size.x != prev_draw_x || drawable_size.y != prev_draw_y)
		compute_checkerboard();

	if (game_mode == 1 || game_mode == 2) {
		std::vector< Vertex > vertices;

		auto draw_rectangle = [&vertices](float xmin, float ymin, float xmax, float ymax, glm::u8vec4 const& color) {
			//draw rectangle as two CCW-oriented triangles:
			vertices.emplace_back(Vertex(glm::vec3(xmin, ymax, 0.0f), color, glm::vec2(0.0f, 1.0f)));
			vertices.emplace_back(Vertex(glm::vec3(xmin, ymin, 0.0f), color, glm::vec2(0.0f, 0.0f)));
			vertices.emplace_back(Vertex(glm::vec3(xmax, ymin, 0.0f), color, glm::vec2(1.0f, 0.0f)));

			vertices.emplace_back(Vertex(glm::vec3(xmin, ymax, 0.0f), color, glm::vec2(0.0f, 1.0f)));
			vertices.emplace_back(Vertex(glm::vec3(xmax, ymin, 0.0f), color, glm::vec2(1.0f, 0.0f)));
			vertices.emplace_back(Vertex(glm::vec3(xmax, ymax, 0.0f), color, glm::vec2(1.0f, 1.0f)));;
		};

		// draw checkerboard
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				struct checkerboard_tile tile = checkerboard[i][j];
				draw_rectangle(tile.left, tile.bot, tile.right, tile.top, tile.color);
			}
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(color_texture_program->program);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(vertex_buffer_for_color_texture_program);
		glUniformMatrix4fv(color_texture_program->OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(projection));

		glBindTexture(GL_TEXTURE_2D, white_tex);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STREAM_DRAW); //upload vertices array
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));

		vertices.clear();

		hover_tile.i = -1;
		hover_tile.j = -1;

		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				struct checkerboard_tile tile = checkerboard[i][j];
				if (!(game_mode == 2 && speed_challenge == ACTIVE)) {
					if (playerid == 2) tile = checkerboard[7 - i][7 - j];
					if (checkerboard[i][j].state == BLACK) {
						draw_rectangle(tile.left, tile.bot, tile.right, tile.top, white_color);
						glBindTexture(GL_TEXTURE_2D, black_checker_tex);
					}
					else if (checkerboard[i][j].state == RED) {
						draw_rectangle(tile.left, tile.bot, tile.right, tile.top, white_color);
						glBindTexture(GL_TEXTURE_2D, red_checker_tex);
					}
					else if (checkerboard[i][j].state == BQUEEN) {
						draw_rectangle(tile.left, tile.bot, tile.right, tile.top, white_color);
						glBindTexture(GL_TEXTURE_2D, black_queen_tex);
					}
					else if (checkerboard[i][j].state == RQUEEN) {
						draw_rectangle(tile.left, tile.bot, tile.right, tile.top, white_color);
						glBindTexture(GL_TEXTURE_2D, red_queen_tex);
					}
					glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
					glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STREAM_DRAW); //upload vertices array
					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));
					vertices.clear();
				}

				tile = checkerboard[i][j];
				if (cursor.x >= tile.left && cursor.x < tile.right && cursor.y >= tile.bot && cursor.y < tile.top) {
					hover_tile.i = i;
					hover_tile.j = j;
				}
				if (click.x >= tile.left && click.x < tile.right && click.y >= tile.bot && click.y < tile.top) {
					struct Selection selected_tile;
					selected_tile.i = i;
					selected_tile.j = j;
					bool already = false;
					for (auto old_tile : selected_tiles) {
						if (old_tile.i == i && old_tile.j == j)
							already = true;
					}
					if (!already)
						selected_tiles.push_back(selected_tile);
					click.x = 0.0f;
					click.y = 0.0f;
				}
			}
		}

		if (!selected_tiles.empty()) {
			for (auto selected_tile : selected_tiles) {
				struct checkerboard_tile tile = checkerboard[selected_tile.i][selected_tile.j];
				draw_rectangle(tile.left, tile.bot, tile.right, tile.top, red_tinge);
			}

			glBindTexture(GL_TEXTURE_2D, white_tex);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STREAM_DRAW); //upload vertices array
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));
			vertices.clear();
		}

		if (hover_tile.i >= 0 && hover_tile.j >= 0) {
			struct checkerboard_tile tile = checkerboard[hover_tile.i][hover_tile.j];
			draw_rectangle(tile.left, tile.bot, tile.right, tile.top, yellow_tinge);

			glBindTexture(GL_TEXTURE_2D, white_tex);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STREAM_DRAW); //upload vertices array
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));
			vertices.clear();
		}
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	if (game_mode == 2) {
		glDisable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ZERO);
		DrawLines lines(projection);

		auto draw_text = [&](glm::vec2 const& at, std::string const& text, float H) {
			lines.draw_text(text,
				glm::vec3(at.x, at.y, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0x00, 0x00, 0x00, 0x00));
			float ofs = 2.0f / drawable_size.y;
			lines.draw_text(text,
				glm::vec3(at.x + ofs, at.y + ofs, 0.0),
				glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
				glm::u8vec4(0x00, 0xaa, 0x11, 0x00));
		};

		if (speed_challenge == ACTIVE) {
			float tilesize = drawsize.y / 10.0f;
			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < 8; j++) {
					if (speedboard[i][j] > 0 && speedboard[i][j] < 5) {
						struct checkerboard_tile tile = checkerboard[i][j];
						draw_text(glm::vec2(tile.left + tilesize / 3.0f, tile.bot + tilesize / 3.0f), std::to_string(speedboard[i][j]), tilesize / 2.0f);

					}
					if (playerid != player_turn) {
						if (speedboard[i][j] > 0) {
							struct checkerboard_tile tile = checkerboard[i][j];
							draw_text(glm::vec2(tile.left + tilesize / 3.0f, tile.bot + tilesize / 3.0f), std::to_string(speedboard[i][j]), tilesize / 2.0f);
						}
					}
				}
			}
			draw_text(glm::vec2(drawsize.x / 2.0f, tilesize / 3.0f), std::to_string(30.0f - speed_timer), 32.0f);
		}
		else if (speed_challenge == COUNTDOWN) {
			draw_text(glm::vec2(drawsize.x / 2.0f - 80.0f, drawsize.y / 2.0f + 64.0f), "Speed challenge begins in", 32.0f);
			draw_text(glm::vec2(drawsize.x / 2.0f - 64.0f, drawsize.y / 2.0f), std::to_string(countdown_timer), 48.0f);
		}
		else if (speed_challenge == WAITING) {
			draw_text(glm::vec2(drawsize.x / 2.0f - 64.0f, drawsize.y / 2.0f), "Finished, waiting for opponent...", 24.0f);
		}
		

	}

	GL_ERRORS();
}
