#include "command_line.h"
#include "git_revision.h"
#include "search_algorithm.h"

#include "task_utils/task_properties.h"
#include "tasks/root_task.h"
#include "utils/logging.h"
#include "utils/system.h"
#include "utils/timer.h"

#include <cassert>
#include <iostream>

#include "pr2/pr2.h"

using namespace std;
using utils::ExitCode;

int main(int argc, const char **argv) {
    try {
        if (argc == 2 &&
            static_cast<string>(argv[1]) == "--internal-git-revision") {
            // We handle this option before registering event handlers to avoid
            // printing peak memory on exit.
            cout << g_git_revision << endl;
            exit(0);
        }
        utils::register_event_handlers();

        if (argc < 2) {
            utils::g_log << get_revision_info() << endl;
            utils::g_log << get_usage(argv[0]) << endl;
            utils::exit_with(ExitCode::SEARCH_INPUT_ERROR);
        }

        shared_ptr<AbstractTask> task;
        bool unit_cost = false;
        if (static_cast<string>(argv[1]) != "--help") {
            utils::g_log << get_revision_info() << endl;
            utils::g_log << "reading input..." << endl;
            tasks::read_root_task(cin);
            task = tasks::g_root_task;
            /*
              TODO once we get rid of g_root_task, the two lines above should
              be replaced by something like
                  task = tasks::read_task(cin)
            */
            utils::g_log << "done reading input!" << endl;
            const AbstractTask &task = *tasks::g_root_task;
            PR2TaskProxy *task_proxy = new PR2TaskProxy(task, new PR2State(task.get_initial_state_values()));

            PR2.proxy = task_proxy;
        }

        std::vector<std::string> args(argv, argv + argc);
        bool parsed = PR2.check_options(args);
        if (!parsed) 
            throw std::invalid_argument( "Parsing Failed" );

        utils::Timer search_timer;
        PR2.run_pr2();
        search_timer.stop();
        utils::g_timer.stop();

        // search_algorithm->save_plan_if_necessary();
        // search_algorithm->print_statistics();
        // utils::g_log << "Search time: " << search_timer << endl;
        // utils::g_log << "Total time: " << utils::g_timer << endl;

        // SearchStatus search_status = search_algorithm->get_status();
        // ExitCode exitcode;
        // switch (search_status) {
        // case SearchStatus::SOLVED:
        //     exitcode = ExitCode::SUCCESS;
        //     break;
        // case SearchStatus::TIMEOUT:
        //     exitcode = ExitCode::SEARCH_UNSOLVED_INCOMPLETE;
        //     break;
        // case SearchStatus::UNSOLVABLE:
        //     exitcode = ExitCode::SEARCH_UNSOLVABLE;
        //     break;
        // case SearchStatus::UNSOLVABLE_WITHIN_BOUND:
        //     exitcode = ExitCode::SEARCH_UNSOLVABLE_WITHIN_BOUND;
        //     break;
        // default:
        //     assert(search_status == SearchStatus::FAILED);
        //     exitcode = ExitCode::SEARCH_UNSOLVED_INCOMPLETE;
        // }
        // exit_with(exitcode);
    } catch (const utils::ExitException &e) {
        /* To ensure that all destructors are called before the program exits,
           we raise an exception in utils::exit_with() and let main() return. */
        return static_cast<int>(e.get_exitcode());
    }
}
