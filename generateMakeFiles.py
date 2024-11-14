import os

# Constants
PATHS_MPLABX_6_20 = [
    'C:\\Program Files\\Microchip\\MPLABX\\v6.20\\mplab_platform\\bin'
]

PATHS_MPLABX_5_35 = [
    'C:\\Program Files (x86)\\Microchip\\MPLABX\\v5.35\\mplab_platform\\bin'
]

# Helper Functions
def add_to_path(path: str):
    path_list = os.environ['PATH'].split(os.pathsep)
    if path not in path_list:
        path_list.insert(0, path)
        os.environ['PATH'] = os.pathsep.join(path_list)
        print(f'Path added: {path}')
    else:
        print(f'Path already exists: {path}')

def remove_from_path(path: str):
    path_list = os.environ['PATH'].split(os.pathsep)
    if path in path_list:
        path_list.remove(path)
        os.environ['PATH'] = os.pathsep.join(path_list)
        print(f'Path removed: {path}')
    else:
        print(f"Path didn't exist: {path}")

def print_paths():
    path_list = os.environ['PATH'].split(os.pathsep)
    for path in path_list:
        print(path)

def generate_make_command():
    os.system('prjMakefilesGenerator .')

# Main Function
def main():
    print('This will take some time, be patient...')

    wd = os.getcwd()

    # Adding needed PATH environment variable for C projects
    for path in PATHS_MPLABX_6_20:
        add_to_path(path)

    print('Generating make files for USB_uC.X')
    os.chdir(f'{wd}/USB_uC.X')
    generate_make_command()

    os.chdir(f'{wd}/USB_uC_Test')
    folders = [name for name in os.listdir() if os.path.isdir(os.path.join('.', name))]

    for project in folders:
        print(f'Generating make files for {project}')
        os.chdir(f'{wd}/USB_uC_Test/{project}')
        generate_make_command()

    # Adding needed PATH environment variable for ASM projects
    for path in PATHS_MPLABX_6_20:
        remove_from_path(path)
    for path in PATHS_MPLABX_5_35:
        add_to_path(path)

    print('Generating make files for USB_uC_16F145X_ASM.X')
    os.chdir(f'{wd}/Assembly Versions/USB_uC_16F145X_ASM.X')
    generate_make_command()

    print('DONE!')

if __name__ == "__main__":
    main()