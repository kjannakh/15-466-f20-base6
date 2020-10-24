
#include "Connection.hpp"

#include "hex_dump.hpp"

#include <chrono>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <random>
#include <vector>

int main(int argc, char **argv) {
#ifdef _WIN32
	//when compiled on windows, unhandled exceptions don't have their message printed, which can make debugging simple issues difficult.
	try {
#endif

	//------------ argument parsing ------------

	if (argc != 2) {
		std::cerr << "Usage:\n\t./server <port>" << std::endl;
		return 1;
	}

	//------------ initialization ------------

	Server server(argv[1]);


	//------------ main loop ------------
	constexpr float ServerTick = 1.0f / 10.0f; //TODO: set a server tick that makes sense for your game

	//server state:

	//per-client state:
	struct PlayerInfo {
		PlayerInfo() {
			static uint32_t next_player_id = 1;
			//name = "Player" + std::to_string(next_player_id);
			player_num = uint8_t(next_player_id);
			next_player_id += 1;
		}
		//std::string name;
		uint8_t player_num;

	};
	std::unordered_map< Connection *, PlayerInfo > players;

	uint8_t checkerboard[8][8];
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if ((i + j) % 2 == 0) {
				if (j < 3)
					checkerboard[i][j] = 1;
				else if (j > 4)
					checkerboard[i][j] = 2;
				else
					checkerboard[i][j] = 0;
			}
			else
				checkerboard[i][j] = 0;
		}
	}
	uint8_t speedboard[64];
	for (int i = 0; i < 64; i++) {
		speedboard[i] = 0;
	}
	uint8_t player_turn = 0;
	uint8_t black_score = 0;
	uint8_t red_score = 0;
	uint8_t game_mode = 0;
	uint8_t black_ready = 0;
	uint8_t red_ready = 0;
	uint8_t turn_count = 0;
	uint32_t black_time = 0;
	uint32_t red_time = 0;
	bool start_speed_challenge = false;
	std::vector< int > rv;
	for (int i = 0; i < 64; i++) {
		rv.push_back(i);
	}
	// shuffle code from https://en.cppreference.com/w/cpp/algorithm/random_shuffle
	std::random_device rd;
	std::mt19937 g(rd());

	while (true) {
		static auto next_tick = std::chrono::steady_clock::now() + std::chrono::duration< double >(ServerTick);
		//process incoming data from clients until a tick has elapsed:
		while (true) {
			auto now = std::chrono::steady_clock::now();
			double remain = std::chrono::duration< double >(next_tick - now).count();
			if (remain < 0.0) {
				next_tick += std::chrono::duration< double >(ServerTick);
				break;
			}
			server.poll([&](Connection *c, Connection::Event evt){
				if (evt == Connection::OnOpen) {
					//client connected:

					//create some player info for them:
					players.emplace(c, PlayerInfo());
						

				} else if (evt == Connection::OnClose) {
					//client disconnected:

					//remove them from the players list:
					auto f = players.find(c);
					assert(f != players.end());
					players.erase(f);


				} else { assert(evt == Connection::OnRecv);
					//got data from client:
					//std::cout << "got bytes:\n" << hex_dump(c->recv_buffer); std::cout.flush();

					//handle messages from client:
					//TODO: update for the sorts of messages your clients send
					while (c->recv_buffer.size() >= 68) {
						//expecting five-byte messages 'b' (left count) (right count) (down count) (up count)
						char type = c->recv_buffer[0];
						if (type != 'b') {
							std::cout << " message of non-'b' type received from client!" << std::endl;
							//shut down client connection:
							c->close();
							return;
						}
						if (c->recv_buffer.size() < 68) break;

						if (c->recv_buffer[1] == game_mode) {

							if (game_mode == 0) {
								uint8_t id = c->recv_buffer[2];
								uint8_t ready = c->recv_buffer[3];
								if (id == 1) black_ready = ready;
								else if (id == 2) red_ready = ready;
							}
							else if (game_mode == 1) {
								uint8_t id = c->recv_buffer[2];
								if (id == player_turn) {
									if (id == 1) black_score += c->recv_buffer[3];
									else if (id == 2) red_score += c->recv_buffer[3];
									for (int i = 0; i < 8; i++) {
										for (int j = 0; j < 8; j++) {
											checkerboard[i][j] = c->recv_buffer[8 * i + j + 4];
										}
									}

									if (player_turn == 1) player_turn = 2;
									else if (player_turn == 2) player_turn = 1;

									turn_count++;
									if (turn_count == 8) {
										turn_count = 0;
										start_speed_challenge = true;
										std::shuffle(rv.begin(), rv.end(), g);
										for (int idx = 0; idx < 5; idx++) {
											speedboard[rv[idx]] = idx + 1;
										}
									}
								}
							}
							else if (game_mode == 2) {
								uint8_t id = c->recv_buffer[2];
								uint32_t time = c->recv_buffer[4];
								time <<= 8;
								time += c->recv_buffer[5];
								time <<= 8;
								time += c->recv_buffer[6];
								time <<= 8;
								time += c->recv_buffer[7];

								if (id == 1) black_time = time;
								else if (id == 2) red_time = time;
							}
						}

						c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin() + 68);
					}
				}
			}, remain);
		}

		//update current game state
		//TODO: replace with *your* game state update
		if (game_mode == 0) {
			if (black_ready && red_ready) {
				game_mode = 1;
				player_turn = 1;
				black_score = 0;
				red_score = 0;
			}
		}
		else if (game_mode == 1) {
			if (black_score == 12 || red_score == 12) {
				for (int i = 0; i < 8; i++) {
					for (int j = 0; j < 8; j++) {
						if ((i + j) % 2 == 0) {
							if (j < 3)
								checkerboard[i][j] = 1;
							else if (j > 4)
								checkerboard[i][j] = 2;
							else
								checkerboard[i][j] = 0;
						}
						else
							checkerboard[i][j] = 0;
					}
				}
				game_mode = 0;
				black_ready = 0;
				red_ready = 0;
			}
		}
		else if (game_mode == 2) {
			if (black_time > 0 && red_time > 0) {
				if (black_time != red_time) {
					if (black_time < red_time) {
						player_turn = 1;
					}
					else {
						player_turn = 2;
					}
				}
				
				game_mode = 1;
				black_time = 0;
				red_time = 0;
				for (int i = 0; i < 64; i++) {
					speedboard[i] = 0;
				}
			}
		}

		//send updated game state to all clients
		//TODO: update for your game state
		for (auto &[c, player] : players) {
			//send an update starting with 'm', a 24-bit size, and a blob of text:
			c->send('m');
			c->send(player.player_num);
			c->send(game_mode);
			c->send(player_turn);
			c->send(black_score);
			c->send(red_score);
			if (game_mode == 0 || game_mode == 1) {
				for (int i = 0; i < 8; i++) {
					for (int j = 0; j < 8; j++) {
						c->send(checkerboard[i][j]);
					}
				}
			}
			else {
				for (int i = 0; i < 64; i++) {
					c->send(speedboard[i]);
				}
			}
			//std::printf("%u, %u, %u, %u\n", game_mode, player_turn, black_score, red_score);
			//c->send_buffer.insert(c->send_buffer.end(), status_message.begin(), status_message.end());
		}
		if (start_speed_challenge) {
			start_speed_challenge = false;
			game_mode = 2;
		}

	}


	return 0;

#ifdef _WIN32
	} catch (std::exception const &e) {
		std::cerr << "Unhandled exception:\n" << e.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "Unhandled exception (unknown type)." << std::endl;
		throw;
	}
#endif
}
