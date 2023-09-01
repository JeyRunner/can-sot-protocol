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
    {
      cli.add_argument(help(showHelp));
      genProtocolSpecCommand.addToCli(cli);
      genProtocolCodeCommand.addToCli(cli);
      string  s;
      bool b;
      cli.add_argument(
              lyra::command("test", [this](const lyra::group & g) { cout << "did test" << endl; })
                      .help("Generate a code (c++ header file) from a protocol def file.")
                      .add_argument(lyra::help(showHelp))
                      .add_argument(
                              lyra::opt(s, "filess") // arg
                                      .name("-fa")
                                      .name("--def-fisdfle")
                                      .required()
                                      .help("Thdfe input protocol def file name."))
                      .add_argument(
                              lyra::opt(b) // arg
                                      .name("-b")
                                      .name("--bbbb")
                                      .help("Thdfe input protocol def file name.")))
                                      ;
    }


    bool parse(int argc, const char **argv) {
      auto cli_result = cli.parse({argc, argv});
      if (!cli_result){
        std::cerr << "Error in command line: " << cli_result.errorMessage() << std::endl;
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

      void addToCli(lyra::cli & cli) {
        cli.add_argument(
            lyra::command("genProtocolSpec", [this](const lyra::group & g) { this->doCmd(g); })
                .help("Generate a protocol definition file from a protocol spec file.")
                //.add_argument(lyra::help(showHelp))
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
              exit(1);
          }
              // need to do additional check due to bug in lyra
          else if (inputSpecFileName.empty() or outputDefFileName.empty()) {
              cout << "Error: not all required arguments are provided" << endl;
              cout << g << endl;
              exit(1);
          }
          else {
              doCommand = true;
          }
      }
    } genProtocolSpecCommand;



    struct GenProtocolCodeCommand {
        bool doCommand = false;
        bool showHelp = false;
        string inputDefFileName;
        string outputHeaderFileName;
        bool forMaster = false;

    private:
        bool args_forMaster = false;
        bool args_forClient = false;
    public:


        void addToCli(lyra::cli & cli) {
            cli.add_argument(
                    lyra::command("genCode", [this](const lyra::group & g) { this->doCmd(g);})
                            .help("Generate a code (c++ header file) from a protocol def file.")
                            .add_argument(lyra::help(showHelp))
                            .add_argument(
                                    lyra::opt(inputDefFileName, "file") // arg
                                            .name("-f")
                                            .name("--def-file")
                                            .required()
                                            .help("The input protocol def file name."))
                            .add_argument(
                                    lyra::opt(outputHeaderFileName, "file")
                                            .name("-o")
                                            .name("--output-header-file")
                                            .required()
                                            .help("The output c++ header file name."))
                            .add_argument(
                                    lyra::opt(args_forMaster)
                                            .name("-m")
                                            .name("--master")
                                            .optional()
                                            .help("Should the code be generated for the master."))
                            .add_argument(
                                    lyra::opt(args_forClient)
                                            .name("--client")
                                            .optional()
                                            .help("Should the code be generated for the client."))
                                            );

        }
        void doCmd(const lyra::group & g) {
            if (showHelp) {
                cout << g << endl;
                exit(1);
            }
            // need to do additional check due to bug in lyra
            else if (inputDefFileName.empty() or inputDefFileName.empty()) {
                cout << "Error: not all required arguments are provided" << endl;
                cout << g << endl;
                exit(1);
                return;
            }
            if (!args_forMaster && !args_forClient) {
                cout << "Error: not specified if generate code for master or client, use either '--master' or '--client'" << endl;
                exit(1);
                return;
            }
            if (args_forMaster && args_forClient) {
                cout << "Error: just select one target to generate code for, use either '--master' or '--client'" << endl;
                exit(1);
                return;
            }
            forMaster = args_forMaster;
            doCommand = true;

        }
    } genProtocolCodeCommand;



};