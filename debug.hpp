#ifndef DEFINES_HPP
    #define DEFINES_HPP

    #if DEBUG
        #include <iostream>
        #define D_OUT(x) std::cout << __FILE__ << " : " << __LINE__ << " [DEBUG CONSOLE] " << x << std::endl;

    #else

        #define D_OUT(x)

    #endif

#endif
