#ifndef TEST_MACROS_HPP
#define TEST_MACROS_HPP

#define REINITIALISE()                                          \
    do {                                                        \
        poise::runtime::memory::intialiseStringInterning();     \
        poise::runtime::memory::Gc::instance().initialise();    \
    } while (false)

#endif // #ifndef TEST_MACROS_HPP

