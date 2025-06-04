// Copyright (c) 2022-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\Viewer.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

// Viewer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "vesper.h"

#include "ViewerApp.h"

#include <cstdlib>
#include <stdexcept>

VESPERENGINE_USING_NAMESPACE

int main()
{
	Config cfg;
    cfg.WindowName = "Vesper Viewer";
	cfg.WindowWidth = 800;
	cfg.WindowHeight = 600;

    cfg.MaxEntities = 1000;
    cfg.MaxComponentsPerEntity = 32;
    
    ViewerApp app(cfg);

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
