#pragma once

void *Xenon_LoadLibrary (const char *filename); 
const char *Xenon_LibraryError(void); 
void *Xenon_LoadFunction(void *handle, char *symbol); 
int Xenon_UnloadLibrary (void *handle);
