FIND_PACKAGE(Threads)

# define some compiler settings

MESSAGE( STATUS "CMAKE_CXX_COMPILER_ID: " ${CMAKE_CXX_COMPILER_ID} )

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    MESSAGE( STATUS "using clang settings" )
    set(SHARED_COMPILE_OPTIONS
        -Wall -Wextra -Werror -Wunused
        -stdlib=libc++
        -ftemplate-depth=1024
        )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    MESSAGE( STATUS "using gnu settings" )
    set(SHARED_COMPILE_OPTIONS
        -Wall -Wextra -Werror -Wunused
        )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    MESSAGE( STATUS "using msvc settings" )
    set(SHARED_COMPILE_OPTIONS
        /W4 /WX
        /wd4503 # truncated symbol
        /wd4702 # unreachable code
        /bigobj
        /DUNICODE /D_UNICODE # it is a new millenium
        )
endif()

set(SHARED_COMPILE_FEATURES
    cxx_auto_type
    cxx_nullptr
    cxx_decltype
    cxx_lambdas
    cxx_lambda_init_captures
    cxx_generic_lambdas
    cxx_range_for
    cxx_return_type_deduction
    cxx_right_angle_brackets
    cxx_rvalue_references
    cxx_static_assert
    cxx_strong_enums
    cxx_thread_local
    cxx_trailing_return_types
    cxx_alias_templates
    cxx_uniform_initialization
    cxx_user_literals
    cxx_variadic_templates
    cxx_template_template_parameters
    )
