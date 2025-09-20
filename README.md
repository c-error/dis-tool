# Display Tool

🎨 Display Tool (dist) is a simple tool for control moniter `color`, `brightness`, `power-off`, etc. for Windows system.

## Features:
- **Color** – adjust monitor rgb color `%` with cuntom color `%`.
- **Brightness** - adjust monitor brightness `%` with cuntom brightness `%`.
- **Night-Light** - preset rgb config for night-light mode.
- **Power-Off** - can turn off display using simple CLI.

## Usage:
- Run:
    ```sh
    > dist.exe help                 # print help
    > dist.exe off                  # turn off display
    > dist.exe nit                  # toggle night mode
    > dist.exe rst                  # reset color to default
    > dist.exe rgb 85 100 100       # adjust display color within 0-100% range
    > dist.exe brit 55              # adjust display brightness within 0-100% range
    > dist.exe setn 100 55 10       # set night light color within 0-100% range
    > dist.exe setd 100 100 100     # set default color within 0-100% range

    ```

## Build:
- **Install Dependencies:**
    - Download and install [MSYS2](https://www.msys2.org/).
    - Open the **mingw64.exe** shell (from the MSYS2 Start Menu).
    - Run the following commands:
        ```sh
        $ pacman -Syu        # Update system packages (MSYS2 may restart)
        $ pacman -Syu        # Run again after restart
        $ pacman -S --needed base-devel mingw-w64-x86_64-toolchain
        ```

- **Set Environment Variables:**
    - Open the **Environment Variables**:
        ```sh
        > rundll32.exe sysdm.cpl,EditEnvironmentVariables   # run in Command Prompt
        ```
    - Add the following paths to your **Path** variable:
        ```
        C:\msys64\mingw64\bin\
        C:\msys64\usr\bin\
        ```

- **Verify Installation:**
    - Run these commands to confirm:
        ```
        > gcc --version
        > g++ --version
        ```
    - If you see version output, your installation was successful 👍

- **Build the Project:**
    - Just run `build.bat`
    - The file `cal.exe` will appear inside the Source folder.

## Project Structure:
The project has the following structure:

```
.
│
├── build.bat
├── main.c
├── icon.ico
├── icon.rc
├── LICENSE
└── README.md
```

## Contributing:
Feel free to contribute to this project by submitting pull requests or reporting issues. Your contributions are greatly appreciated!

## License:
This project is licensed under the [MIT License](LICENSE).

😄 Happy coding!
