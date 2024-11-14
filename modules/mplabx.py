import os
from typing import Union

class MPLABX:
    class Config:
        def __init__(self, name: str, defines: dict[str,str]):
            self.name = name
            self.defines = defines

    class Project:
        def __init__(self, prj_path: str, config: 'MPLABX.Config'):
            self.prj_loc = prj_path
            self.config = config

    class BuildJob:
        def __init__(self, project: 'MPLABX.Project', output_path: str):
            self.project = project
            self.output_path = output_path

    def __print_build_start(output_path: str):
        message = f"""
\n\n--------------------------------------------------------------------------
    BUILDING \"{output_path}\"
--------------------------------------------------------------------------
"""
        print(message)

    def __generate_make_line(config_name: str, defines: dict[str,str], asm: bool, warn_level: Union[int, str]) -> str:
        command = f'make -s -B CONF={config_name}'
        if not asm:
            if warn_level != 'OFF':
                command += f' MP_EXTRA_LD_PRE="--warn={warn_level}" MP_EXTRA_CC_PRE="--warn={warn_level}'
            else:
                command += f' MP_EXTRA_LD_PRE="-w" MP_EXTRA_CC_PRE="-w'
        else:
            if warn_level != 'OFF':
                command +=  f' MP_EXTRA_AS_PRE="-w{warn_level}'
            else:
                command +=  f' MP_EXTRA_AS_PRE="-w '
        if defines:
            command += ' '
            items = len(defines)
            for index, (key, value) in enumerate(defines.items()):
                command += f'-D{key}'
                if value:
                    command += f'={value}'
                if index != (items - 1):
                    command += ' '
        return command + '"'
        
    @classmethod
    def build(cls, working_dir: str, job: 'MPLABX.BuildJob', asm: bool, warn_level: Union[int, str]):
        cls.__print_build_start(job.output_path)

        prj_loc = os.path.abspath(f'{working_dir}/{job.project.prj_loc}')
        print(f'Directory change to: {prj_loc}')
        os.chdir(prj_loc)

        command = cls.__generate_make_line(job.project.config.name, job.project.config.defines, asm, warn_level)
        print(f'Running: {command}\n')
        os.system(command)
        
        input_hex_file = os.path.abspath(f'dist/{job.project.config.name}/production/{os.path.basename(job.project.prj_loc)}.production.hex')
        output_hex_file = os.path.abspath(f'{working_dir}/HEX Files/{job.output_path}')
        output_dir = os.path.abspath(os.path.dirname(output_hex_file))
        print(f'Hex file: {input_hex_file}')
        print(f'Saved file: {output_hex_file}')
        if not os.path.isdir(output_dir):
            print(f'Making "{output_dir}" directory.')
            os.makedirs(output_dir)
        if os.path.exists(output_hex_file):
            print(f'File already exists, removing file.')
            os.remove(output_hex_file)
        if os.path.isfile(input_hex_file):
            print(f'Copying and renaming input hex to output hex location.')
            os.rename(input_hex_file, output_hex_file)
        print('\nDONE!\n')