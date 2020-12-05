#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <string>
#include <vector>

std::string testDataPath;

int main(int argc, char* argv[])
{
  Catch::Session session; // There must be exactly one instance

  // Build a new parser on top of Catch's
  using namespace Catch::clara;
  auto cli =
      session.cli() | Opt(testDataPath, "test data dir")["--test-data-dir"];

  // Now pass the new composite back to Catch so it uses that
  session.cli(cli);

  // Let Catch (using Clara) parse the command line
  auto const returnCode = session.applyCommandLine(argc, argv);
  if (returnCode != 0) // Indicates a command line error
    return returnCode;

  return session.run();
}
