#ifndef MINI_READLINE_H
#define MINI_READLINE_H

#include <string>
#include <vector>

class MiniReadline
{
   public:
      MiniReadline();
      virtual ~MiniReadline();

      bool promptForCommand(const std::string& prompt);
      char getNextCmdLineChar();
   private:
      std::string lastCommand;
      std::string::const_iterator streamPosition;      
};

#endif // MINI_READLINE_H
