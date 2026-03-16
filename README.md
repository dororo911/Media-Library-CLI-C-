# Media Library CLI (C++)

Simple console app for managing a small library: users log in, then use role-specific menus to browse, loan, and administer media items.

## Features
- Login with roles: Client, Admin, Super Admin
- Menu-driven navigation for borrowing/returning and library management
- Library data persisted in `bibliotheque.txt`; users in `utilisateurs.txt`

## Requirements
- C++17 compiler (uses `<filesystem>`)
- Windows or any platform with a compatible compiler

## Build
```sh
g++ -std=c++17 -O2 main.cpp media.cpp menus.cpp users.cpp utils.cpp -o app
```

## Run
Keep `bibliotheque.txt` and `utilisateurs.txt` alongside the binary, then run:
```sh
./app
```

## Project Layout
- `main.cpp` – entry point showing the welcome screen and routing to menus
- `media*.{h,cpp}` – media entity handling
- `users*.{h,cpp}` – user accounts and login
- `menus*.{h,cpp}` – role-based menus
- `utils*.{h,cpp}` – helpers
- `bibliotheque.txt`, `utilisateurs.txt` – sample data files
