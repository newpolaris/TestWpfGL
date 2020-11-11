#pragma once

#define EXPORT extern "C" __declspec(dllexport)

EXPORT int GLCreate(const void* handle);
EXPORT int Render();
