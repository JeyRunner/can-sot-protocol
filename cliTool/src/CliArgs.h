#pragma once

#include <lyra/lyra.hpp>

using namespace lyra;
using namespace std;


class CliArgs {
  public:
    lyra::cli cli;

  public:
    bool showHelp = false;


    CliArgs()
    : genProtocolSpecCommand(cli)
    {
      cli.add_argument(help(showHelp));
    }


    bool parse(int argc, const char **argv) {
      auto cli_result = cli.parse({argc, argv});
      if (!cli_result){
        std::cerr << "Error in command line: " << cli_result.message() << std::endl;
        exit(1);
      }
      if (showHelp){
        std::cout << endl << cli << std::endl;
        exit(0);
      }
    }



    struct GenProtocolSpecCommand {
      bool doCommand = false;
      bool showHelp = false;
      string inputSpecFileName;
      string outputDefFileName;

      GenProtocolSpecCommand(lyra::cli & cli) {
        cli.add_argument(
            lyra::command("genProtocolSpec", [this](const lyra::group & g) { this->doCmd(g); })
                .help("Generate a protocol definition file from a protocol spec file.")
                .add_argument(lyra::help(showHelp))
                .add_argument(
                    lyra::opt(inputSpecFileName, "file") // arg
                        .name("-f")
                        .name("--spec-file")
                        .required()
                        .help("The input protocol spec file name."))
                .add_argument(
                    lyra::opt(outputDefFileName, "file")
                        .name("-o")
                        .name("--output-def-file")
                        .required()
                        .help("The output definition file name.")));
      }
      void doCmd(const lyra::group & g) {
        if (showHelp) {
          cout << g << endl;
        }
        else {
          doCommand = true;
        }
      }
    } genProtocolSpecCommand;


};