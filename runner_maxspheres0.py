#!/usr/bin/env python3
# config_test_runner.py

import re
import os
import subprocess
import time
from itertools import product

# Path configurations
#PROJECT_DIR = r"C:\Users\Tomás\Documents\GitHub\EDAA"
PROJECT_DIR = r"d:\Universidade\Y4S2\EDAA\EDAA"
CONFIG_PATH = os.path.join(PROJECT_DIR, "src", "config.h")
BUILD_DIR = os.path.join(PROJECT_DIR, "build")
EXE_PATH = os.path.join(BUILD_DIR, "EDAA.exe")

def modify_config(param_dict):
    """
    Modify the config.h file with the given parameters
    
    Args:
        param_dict: Dictionary of parameter names and their new values
    """
    print(f"Updating config with: {param_dict}")
    
    with open(CONFIG_PATH, 'r') as f:
        content = f.read()
    
    # Update each parameter in the config file
    for param, value in param_dict.items():
        # Regular expression to match the parameter definition
        pattern = rf'const\s+(?:int|unsigned int|bool)\s+{param}\s*=\s*[^;]+;'
        
        # Determine the type of the parameter
        type_match = re.search(rf'const\s+(int|unsigned int|bool)\s+{param}', content)
        if type_match:
            param_type = type_match.group(1)
            # Create replacement with the correct type
            replacement = f'const {param_type} {param} = {value};'
            content = re.sub(pattern, replacement, content)
            print(f"  - Set {param} = {value}")
    
    # Write updated content back to file
    with open(CONFIG_PATH, 'w') as f:
        f.write(content)
    
    print("Config file updated successfully")

def compile_project():
    """
    Compile the project using CMake
    """
    print("Compiling project...")
    try:
        # Run CMake to generate build files if needed
        if not os.path.exists(BUILD_DIR):
            os.makedirs(BUILD_DIR)
            subprocess.run(["cmake", "-G", "Unix Makefiles", "-S", PROJECT_DIR, "-B", BUILD_DIR], 
                          check=True, capture_output=True, text=True)
        
        # Build the project
        result = subprocess.run(["make"], 
                              cwd=BUILD_DIR,  # Run make in the build directory
                              check=True, 
                              capture_output=True,
                              text=True)
        print("Compilation successful!")
        return True
    except subprocess.CalledProcessError as e:
        print(f"Compilation error: {e}")
        print(f"Output: {e.stdout}")
        print(f"Error: {e.stderr}")
        return False

def run_executable(timeout=60):
    """
    Run the executable with a timeout
    """
    print(f"Running {EXE_PATH}...")
    try:
        # Headless mode needs some specific environment variables
        env = os.environ.copy()
        
        # Run with timeout to prevent hanging
        process = subprocess.Popen([EXE_PATH], env=env)
        
        # Wait for process to complete or timeout
        try:
            process.wait(timeout=timeout)
            print("Execution completed successfully")
            return True
        except subprocess.TimeoutExpired:
            print(f"Execution timed out after {timeout} seconds, killing process")
            process.kill()
            return False
    except Exception as e:
        print(f"Error running executable: {e}")
        return False

def run_experiments():
    """
    Run experiments with different parameter combinations
    """
    # Base parameters all experiments will use
    base_params = {
        'COLLECTSTATS': 1, 
        'DEBUG': 0,
        'USEPREBUILT': 0,
        'MAXSPHERESPERNODE': 0,
    }
    
    # Define screen resolutions with proper aspect ratios
    resolutions = [
        {'SCR_WIDTH': 800, 'SCR_HEIGHT': 600},
    ]
    
    # Define sphere counts to test
    sphere_counts = [10, 25, 50]

    quality_levels = [
        {'NUMSAMPLES': 4, 'MAXRAYSDEPTH': 4},
        {'NUMSAMPLES': 16, 'MAXRAYSDEPTH': 8},
        {'NUMSAMPLES': 32, 'MAXRAYSDEPTH': 16}
    ]
    
    # Common parameters for both with and without octree
    common_combinations = list(product(
        sphere_counts,
        quality_levels,
        resolutions
    ))
    
    # Parameters specific to octree configuration
    octree_params = {
        'MAXDEPTH': [1, 5, 6, 7, 8, 9],
    }
    
    octree_combinations = list(
        octree_params['MAXDEPTH'],
    )
    
    # Create all experiment configurations
    all_experiments = []
    
    # 2. Experiments with octree and varying octree parameters
    for num_spheres, quality, resolution in common_combinations:
        for max_depth in octree_combinations:
            experiment = base_params.copy()
            experiment.update({
                'USEOCTREE': 1,
                'NUMSPHERES': num_spheres,
                'MAXDEPTH': max_depth
            })
            experiment.update(resolution)
            experiment.update(quality)
            all_experiments.append(experiment)
    
    print(f"Running {len(all_experiments)} parameter combinations")
    
    # Run each combination
    for i, params in enumerate(all_experiments):
        print(f"\nExperiment {i+1}/{len(all_experiments)}")
        
        # Update config
        modify_config(params)
        
        # Compile and run
        if compile_project():
            time.sleep(1)
            
            success = run_executable(timeout=500) 
            
            if success:
                print("Waiting 2 seconds before next experiment...")
                time.sleep(2)
            else:
                print("Experiment failed, continuing to next one")
        else:
            print("Skipping execution due to compilation failure")

if __name__ == "__main__":
    print("EDAA Performance Testing Script")
    print("===============================")
    run_experiments()
    print("\nAll experiments completed!")