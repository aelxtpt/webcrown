function(add_c2a_module)

    set(modules_prebuilt_directory ${CMAKE_BINARY_DIR}/modules)
    message("Creating modules prebuild directory: ${modules_prebuilt_directory}")

    # Create output directory for modules
    file(MAKE_DIRECTORY ${modules_prebuilt_directory})

    set(modules_prebuit_out)

    foreach(_arg ${ARGN})

        # Put the base filename in variable module_name
        get_filename_component(barename ${_arg} NAME)
        string(REPLACE ".cpp" "" module_name ${barename})

        set(_output_file ${modules_prebuilt_directory}/${module_name}.pcm)
        set(_input_file ${_arg}) # Full path of file

        list(APPEND modules_prebuit_out ${_output_file})

        set(_command_exec clang++ -std=c++2a -stdlib=libc++ -c ${_input_file} -Xclang -emit-module-interface -o ${_output_file})

        message("Command to execute ${_command_exec}")

        execute_process(
                COMMAND
                ${_command_exec}
                WORKING_DIRECTORY ${modules_prebuilt_directory}
                OUTPUT_VARIABLE _stdout
                ERROR_VARIABLE _stderr
        )

        message("${_stdout}")
        #--debug-output --trace-expand -DVERBOSE=ON
        if(_stderr)
            message("Error on trying generating prebuilt modules ${_stderr}")
        endif()

    endforeach()

    set(modules_prebuit ${modules_prebuit_out} PARENT_SCOPE)

endfunction()