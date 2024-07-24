#include <Pch.h>
#include <game.hpp>

int main()
{
    SetConsoleTitleA("ISLE FREE DMA");
    if (!mem.Init("FortniteClient-Win64-Shipping.exe", true, false)) {
        std::cout << "Failed to initialize DMA" << std::endl;
        return 1;
    }
    system("CLS");

    cache::base = mem.GetBaseDaddy("FortniteClient-Win64-Shipping.exe");

    if (!init()) {
        printf("The gui was not initialized");
        Sleep(3000);
        exit(0);
    }

    if (!mem.GetKeyboard()->InitKeyboard()) {
        printf("The keyboard was not initialized");
    }

    system("CLS");

    std::thread base_thread(local_pointers);
    std::thread actor_thread(actorloop);

    base_thread.detach();
    actor_thread.detach();

    WPARAM result = render_loop();
    exit(0);
}
