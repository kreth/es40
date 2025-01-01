#include "MiniReadline.h"
#include "config.h"

#ifdef HAVE_LIBREADLINE
#  if defined(HAVE_READLINE_READLINE_H)
#    include <readline/readline.h>
#  elif defined(HAVE_READLINE_H)
#    include <readline.h>
#  else /* !defined(HAVE_READLINE_H) */
extern char *readline ();
#  endif /* !defined(HAVE_READLINE_H) */
char *cmdline = NULL;
#else /* !defined(HAVE_READLINE_READLINE_H) */
/* no readline */
#endif /* HAVE_LIBREADLINE */

#ifdef HAVE_READLINE_HISTORY
#  if defined(HAVE_READLINE_HISTORY_H)
#    include <readline/history.h>
#  elif defined(HAVE_HISTORY_H)
#    include <history.h>
#  else /* !defined(HAVE_HISTORY_H) */
extern void add_history ();
extern int write_history ();
extern int read_history ();
#  endif /* defined(HAVE_READLINE_HISTORY_H) */
/* no history */
#endif /* HAVE_READLINE_HISTORY */

// #include <readline/readline.h>
// #include <readline/history.h>

MiniReadline::MiniReadline()
   : lastCommand(""),
     streamPosition(lastCommand.cbegin())
{}

MiniReadline::~MiniReadline() {}

bool MiniReadline::promptForCommand(const std::string& prompt) {
   char *input;
   input = readline(prompt.c_str());
   if(input) {
      add_history(input);
      lastCommand = input;
      streamPosition = lastCommand.cbegin();
      return true;
   } else return false;
}

char MiniReadline::getNextCmdLineChar() {
   if(streamPosition != lastCommand.cend()) {
      return *streamPosition++;
   } else {
      return 0;
   }
}
