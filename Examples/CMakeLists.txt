# SAGE Engine Examples

# Функция для создания executable для примера
function(add_sage_example NAME SOURCE_FILE)
    add_executable(${NAME} ${SOURCE_FILE})
    target_link_libraries(${NAME} PRIVATE SAGE_Engine)
    
    # Копируем assets если они есть
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}_assets")
        add_custom_command(TARGET ${NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}_assets"
            "$<TARGET_FILE_DIR:${NAME}>/assets"
        )
    endif()
    
    # Вывод в bin директорию
    set_target_properties(${NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endfunction()

# Создаём примеры
add_sage_example(Box2DPhysicsDemo Box2DPhysicsDemo.cpp)
add_sage_example(LogConDemo LogConDemo.cpp)
add_sage_example(MusicSystemExample MusicSystemExample.cpp)
add_sage_example(NineSliceExample NineSliceExample.cpp)
add_sage_example(OptimizationSystemsExample OptimizationSystemsExample.cpp)
add_sage_example(ParticleSystemDemo ParticleSystemDemo.cpp)
add_sage_example(ParticleSystemSimpleDemo ParticleSystemSimpleDemo.cpp)
add_sage_example(PhysicsSandbox PhysicsSandbox.cpp)
add_sage_example(PlayerInventoryDemo PlayerInventoryDemo.cpp)
add_sage_example(PluginDemo PluginDemo.cpp)
add_sage_example(RPGSystemsDemo RPGSystemsDemo.cpp)
add_sage_example(ScreenEffectsExample ScreenEffectsExample.cpp)
add_sage_example(TilemapLoaderExample TilemapLoaderExample.cpp)
add_sage_example(TilemapOrientationsDemo TilemapOrientationsDemo.cpp)
add_sage_example(UISystemTest UISystemTest.cpp)
add_sage_example(UISystemTestNew UISystemTestNew.cpp)
add_sage_example(UtilitySystemsExample UtilitySystemsExample.cpp)

message(STATUS "Added 17 example projects")
