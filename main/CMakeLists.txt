file(GLOB_RECURSE SOURCES OLED/*.c
					webserver/*.c
					ARM/*.c
					*.c
					util/*.c
					util/cJSON/*.c
					)
set(include_dirs
	OLED/include
	util/cJSON
	util/
	ARM/include
	webserver/include
	.)
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${include_dirs})
