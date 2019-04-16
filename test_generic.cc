#include "catch2.hh"
#include <unistd.h>
#include <string>
#include <sstream>
#include <cstdio>
#include <iostream>
#include <utility>


constexpr int BUFFERSIZE=1024;

TEST_CASE("Valgrind pass, checking all other tests and also hidden Valgrind ones",
		  "[all][do the valgrind thing]"){
  char buf[BUFFERSIZE];
  ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf)-1);

  REQUIRE(len != -1);
  if (len != -1) {
	buf[len] = '\0';
  }

  // build a command line to valgrind oneself and then do that
  std::stringstream command;
  command << "valgrind --leak-check=full --error-exitcode=42 "
		  << buf
		  << " -r compact"
		  << " '[all]~[do the valgrind thing],[.valgrind]' 2>&1"; // < also run the otherwise-hidden valgrinding test cases with no requires
  //                    ^ ensure we don't get stuck in a recursive loop and forkbomb everything

  // and now for popen shenanigans, to not clog stdout with
  // messages if everything is actually fine
  // thanks to https://stackoverflow.com/questions/52164723/how-to-execute-a-command-and-get-return-code-stdout-and-stderr-of-command-in-c
  // for explaining how to do this
  std::array<char, 128> output_buffer;
  auto pipe = popen(command.str().c_str(), "r");

  REQUIRE(pipe);

  while (!feof(pipe)){
	if (fgets(output_buffer.data(), 128, pipe) != nullptr){
	  UNSCOPED_INFO(output_buffer.data());
	}
  }

  REQUIRE(pclose(pipe) == EXIT_SUCCESS);
}


TEST_CASE("asserts fail when necessary",
		  "[all]"){
  char buf[BUFFERSIZE];
  ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf)-1);  
  REQUIRE(len != -1);
  if (len != -1) {
	buf[len] = '\0';
  }

  // and now for messy shenanigans. first, get all the crash test names:
  std::stringstream command;
  command << buf << " -l [.crash] | grep '^  [^ ]' | sed 's/^  //g'";
  INFO(command.str());

  // and now do popen shenanigans
  std::array<char, BUFFERSIZE> test_buffer;
  auto pipe = popen(command.str().c_str(), "r");
  REQUIRE(pipe);
  while (!feof(pipe)){
	if (fgets(test_buffer.data(), BUFFERSIZE, pipe) != nullptr){
	  // so now that we have each test name:
	  std::string test = std::string(test_buffer.data());
	  test.pop_back(); // remove newline from the end

	  // another round of popening to make sure the test fails!
	  DYNAMIC_SECTION("assert fails if: " << test){
		std::stringstream subcommand;
		subcommand << buf << " -r compact \"" << test << "\" 2>&1";
		INFO(subcommand.str());

		std::array<char, BUFFERSIZE> output_buffer;
		auto subpipe = popen(subcommand.str().c_str(), "r");
		REQUIRE(subpipe);
		while (!feof(subpipe)){
		  if (fgets(output_buffer.data(), BUFFERSIZE, subpipe) != nullptr){
			UNSCOPED_INFO(output_buffer.data());
		  }
		}
		REQUIRE(pclose(subpipe) != EXIT_SUCCESS);
	  }
	}
  }
  REQUIRE(pclose(pipe) == EXIT_SUCCESS);
}
