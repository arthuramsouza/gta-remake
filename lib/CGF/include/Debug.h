#ifndef CGF_DEBUG_H
#define CGF_DEBUG_H


// se DISABLE_LOGGING não estiver definido, então habilita logging
// foi feita essa lógica de maneira que as mensagens apareçam por padrão e sejam desabilitadas
// caso o usuário queira (mantendo o mesmo comportamento para todos os sistemas de build
#ifndef DISABLE_LOGGING

#include <iostream>
#define DEBUG_MSG(str) do { std::cout << str << '\n'; } while( false )
// no new line
#define DEBUG_MSG_NN(str) do { std::cout << str; } while( false )

#else

#define DEBUG_MSG(str) do { } while ( false )
#define DEBUG_MSG_NN(str) do { } while ( false )

#endif
#endif //CGF_DEBUG_H
