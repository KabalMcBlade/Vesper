// Viewer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "vesper.h"

#include "WindowApp.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

VESPERENGINE_USING_NAMESPACE

int main()
{
// 	glm::vec4 test0 = { 1.0f, 2.0f, 3.0f, 4.0f };
// 	glm::vec4 test1 = { 7.0f, 7.0f, 7.0f, 7.0f };
// 
// 	glm::vec4 test3 = test0;
// 	glm::vec4 testAdd = test0 + test3;

	Config cfg;
    cfg.WindowName = "Vesper Viewer";
	cfg.WindowWidth = 800;
	cfg.WindowHeight = 600;
    
    WindowApp app(cfg);

    try
    {
        app.Run();
    }
    catch (const std::exception& _e)
    {
        std::cerr << _e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
