"""
This python script will build the all of the USB_uC MPLABX projects, producing hex files for all
supported development boards with different crystal options.
The output folder location is in the 'Hex Files' directory.

Prerequisites:
- Install GIT
- Install MPLABX v6.20
    - Device Packages Installed:
        - PIC12-16F1xxx_DFP v1.7.242
        - PIC18Fxxxx_DFP v1.6.159
        - PIC18F-K_DFP v1.13.292
        - PIC18F-J_DFP v1.7.159
- Install XC8 v2.46
- Install MPLABX v5.35 (for assembly version)
- Run "python generateMakeFiles.py" when you first clone the repo.
- Run "git submodule update --init --remote --recursive"
"""

import os
from modules.mplabx import MPLABX


# Constants
XTAL_OPTIONS = [
    ['MHz_12', 'NO_XTAL'],
    ['MHz_12', 'MHz_16', 'NO_XTAL'],
    ['MHz_4', 'MHz_8', 'MHz_12', 'MHz_16', 'MHz_20', 'MHz_24', 'MHz_40', 'MHz_48']
]

PATHS_COMPILER_2_46 = [
    'C:\\Program Files\\Microchip\\xc8\\v2.46\\bin',
    'C:\\Program Files\\Microchip\\xc8\\v2.46\\pic'
]

PATHS_MPLABX_6_20 = [
    'C:\\Program Files\\Microchip\\MPLABX\\v6.20\\gnuBins\\GnuWin32\\bin',
    'C:\\Program Files\\Microchip\\MPLABX\\v6.20\\mplab_platform\\bin'
]

PATHS_MPLABX_5_35 = [
    'C:\\Program Files (x86)\\Microchip\\MPLABX\\v5.35\\gnuBins\\GnuWin32\\bin',
    'C:\\Program Files (x86)\\Microchip\\MPLABX\\v5.35\\mplab_platform\\bin'
]


# Data Classes
class Board:
    def __init__(self, name: str, xtals: list[str] = ['']):
        self.name = name
        self.xtals = xtals

class Config:
    def __init__(self, name: str, fn_start: str, output_path: str, boards: list[Board]):
        self.name = name
        self.fn_start = fn_start
        self.output_path = output_path
        self.boards = boards

class Project:
    def __init__(self, prj_path: str, configs: list[Config]):
        self.prj_path = prj_path
        self.configs = configs


# Build Data
C_BUILD_DATA = [
    Project('USB_uC.X', [
        Config('PIC16F1454', 'USB_uC_145X', 'PIC16F145X', [
            Board('DM164127', XTAL_OPTIONS[0]),
            Board('GENERAL', XTAL_OPTIONS[1]),
            Board('XPRESS')
        ]),
        Config('PIC18F14K50', 'USB_uC_14K50', 'PIC18F14K50', [
            Board('DEV_BOARD'),
            Board('DM164127'),
            Board('GENERAL')
        ]),
        Config('PIC18F24K50', 'USB_uC_24K50', 'PIC18F24K50', [
            Board('GENERAL', XTAL_OPTIONS[1]),
            Board('P_STAR')
        ]),
        Config('PIC18F44J50', 'USB_uC_X4J50', 'PIC18FX4J50', [
            Board('DEV_BOARD', XTAL_OPTIONS[2])
        ]),
        Config('PIC18F45J50', 'USB_uC_X5J50', 'PIC18FX5J50', [
            Board('DEV_BOARD', XTAL_OPTIONS[2])
        ]),
        Config('PIC18F45K50', 'USB_uC_45K50', 'PIC18FX5K50', [
            Board('PICDEM')
        ]),
        Config('PIC18F45K50', 'USB_uC_X5K50', 'PIC18FX5K50', [
            Board('GENERAL', XTAL_OPTIONS[1]),
            Board('P_STAR')
        ]),
        Config('PIC18F46J50', 'USB_uC_X6J50', 'PIC18FX6J50', [
            Board('DEV_BOARD', XTAL_OPTIONS[2])
        ]),
        Config('PIC18F46J53', 'USB_uC_46J53', 'PIC18FX6J53', [
            Board('PIM')
        ]),
        Config('PIC18F46J53', 'USB_uC_X6J53', 'PIC18FX6J53', [
            Board('DEV_BOARD', XTAL_OPTIONS[2]),
            Board('PIC_CLICKER')
        ]),
        Config('PIC18F47J53', 'USB_uC_47J53', 'PIC18FX7J53', [
                Board('PIM')
        ]),
        Config('PIC18F47J53', 'USB_uC_X7J53', 'PIC18FX7J53', [
            Board('DEV_BOARD', XTAL_OPTIONS[2]),
            Board('PIC_CLICKER')
        ]),
        Config('PIC18F4450', 'USB_uC_X450', 'PIC18FX450', [
            Board('DEV_BOARD', XTAL_OPTIONS[2]),
            Board('GENERAL', XTAL_OPTIONS[2])
        ]),
        Config('PIC18F4455', 'USB_uC_X455', 'PIC18FX455', [
            Board('DEV_BOARD', XTAL_OPTIONS[2]),
            Board('GENERAL', XTAL_OPTIONS[2])
        ]),
        Config('PIC18F4458', 'USB_uC_X458', 'PIC18FX458', [
            Board('DEV_BOARD', XTAL_OPTIONS[2]),
            Board('GENERAL', XTAL_OPTIONS[2])
        ]),
        Config('PIC18F4550', 'USB_uC_2550', 'PIC18FX550', [
            Board('MIKROE_647')
        ]),
        Config('PIC18F4550', 'USB_uC_X550', 'PIC18FX550', [
                Board('DEV_BOARD', XTAL_OPTIONS[2]),
                Board('GENERAL', XTAL_OPTIONS[2])
        ]),
        Config('PIC18F4553', 'USB_uC_X553', 'PIC18FX553', [
                Board('DEV_BOARD', XTAL_OPTIONS[2]),
                Board('GENERAL', XTAL_OPTIONS[2])
        ])
    ]),
    Project('USB_uC_Test/Test_145X.X', [
        Config('PIC16F1459', 'Test_145X', 'PIC16F145X', [
            Board('DM164127', XTAL_OPTIONS[0]),
            Board('XPRESS')
        ]),
    ]),
    Project('USB_uC_Test/Test_14K50.X', [
        Config('default', 'Test_14K50', 'PIC18F14K50', [
            Board('DEV_BOARD'),
            Board('DM164127')
        ]),
    ]),
    Project('USB_uC_Test/Test_K50.X', [
        Config('PIC18F24K50', 'Test_K50', 'PIC18F24K50', [
            Board('P_STAR')
        ]),
        Config('PIC18F45K50', 'Test_45K50', 'PIC18FX5K50', [
            Board('PICDEM')
        ]),
        Config('PIC18F45K50', 'Test_K50', 'PIC18FX5K50', [
            Board('P_STAR')
        ]),
    ]),
    Project('USB_uC_Test/Test_J_PART.X', [
        Config('PIC18F44J50', 'Test_X4J50', 'PIC18FX4J50', [
            Board('DEV_BOARD')
        ]),
        Config('PIC18F45J50', 'Test_X5J50', 'PIC18FX5J50', [
            Board('DEV_BOARD')
        ]),
        Config('PIC18F46J50', 'Test_X6J50', 'PIC18FX6J50', [
            Board('DEV_BOARD')
        ]),
        Config('PIC18F46J53', 'Test_46J53', 'PIC18FX6J53', [
            Board('PIM')
        ]),
        Config('PIC18F46J53', 'Test_X6J53', 'PIC18FX6J53', [
            Board('DEV_BOARD'),
            Board('PIC_CLICKER')
        ]),
        Config('PIC18F47J53', 'Test_47J53', 'PIC18FX7J53', [
            Board('PIM')
        ]),
        Config('PIC18F47J53', 'Test_X7J53', 'PIC18FX7J53', [
            Board('DEV_BOARD'),
            Board('PIC_CLICKER')
        ]),
    ]),
    Project('USB_uC_Test/Test_X450.X', [
        Config('PIC18F4450', 'Test_X450', 'PIC18FX450', [
            Board('DEV_BOARD')
        ])
    ]),
    Project('USB_uC_Test/Test_4550_FAMILY.X', [
        Config('PIC18F4455', 'Test_X455', 'PIC18FX455', [
            Board('DEV_BOARD')
        ]),
        Config('PIC18F4458', 'Test_X458', 'PIC18FX458', [
            Board('DEV_BOARD')
        ]),
        Config('PIC18F4550', 'Test_2550', 'PIC18FX550', [
            Board('MIKROE_647')
        ]),
        Config('PIC18F4550', 'Test_X550', 'PIC18FX550', [
            Board('DEV_BOARD')
        ]),
        Config('PIC18F4553', 'Test_X553', 'PIC18FX553', [
            Board('DEV_BOARD')
        ])
    ]),
    Project('USB_uC_Test/Test_145X_ASM.X', [
        Config('PIC16F1459', 'Test_145X', 'ASM', [
            Board('DM164127', XTAL_OPTIONS[0]),
            Board('XPRESS')
        ]),
    ])
]

ASM_BUILD_DATA = [
    Project('Assembly Versions/USB_uC_16F145X_ASM.X', [
        Config('PIC16F1454', 'USB_uC_1454', 'ASM', [
            Board('GENERAL', XTAL_OPTIONS[1]),
            Board('XPRESS')
        ])
    ]),
    Project('Assembly Versions/USB_uC_16F145X_ASM.X', [
        Config('PIC16F1455', 'USB_uC_1455', 'ASM', [
            Board('GENERAL', XTAL_OPTIONS[1]),
        ])
    ]),
    Project('Assembly Versions/USB_uC_16F145X_ASM.X', [
        Config('PIC16F1459', 'USB_uC_1459', 'ASM', [
            Board('DM164127', XTAL_OPTIONS[0]),
            Board('GENERAL', XTAL_OPTIONS[1]),
        ])
    ]),
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

def generate_jobs(build_data: list['Project']) -> list[MPLABX.BuildJob]:
    jobs = []
    for project in build_data:
        for config in project.configs:
            for board in config.boards:
                for xtal in board.xtals:
                    defs = {'BOARD_VERSION': board.name}
                    file_name = f'{config.output_path}/{config.fn_start}_{board.name}'
                    if xtal:
                        defs['XTAL_USED'] = xtal
                        xtal = xtal if xtal == 'NO_XTAL' else xtal.split('_')[1] + xtal.split('_')[0]
                        file_name += f'_{xtal}.hex'
                    else:
                        file_name += '.hex'
                    jobs.append(MPLABX.BuildJob(MPLABX.Project(f'{project.prj_path}', MPLABX.Config(config.name, defs)), file_name))
    return jobs


# Main Function
def main():
    wd = os.getcwd()

    # Adding needed PATH environment variable for C projects
    for path in PATHS_COMPILER_2_46:
        add_to_path(path)    
    for path in PATHS_MPLABX_6_20:
        add_to_path(path)

    jobs = generate_jobs(C_BUILD_DATA)

    for job in jobs:
        MPLABX.build(wd, job, False, 'OFF')

    # Adding needed PATH environment variable for ASM projects
    for path in PATHS_MPLABX_6_20:
        remove_from_path(path)
    for path in PATHS_MPLABX_5_35:
        add_to_path(path)

    jobs = generate_jobs(ASM_BUILD_DATA)

    for job in jobs:
        MPLABX.build(wd, job, True, 2)


if __name__ == "__main__":
    main()