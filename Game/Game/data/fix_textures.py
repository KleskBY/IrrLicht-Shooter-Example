import os

def process_mtl_file(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    new_lines = []
    current_material = None
    map_kd_exists = False

    for line in lines:
        if line.startswith('newmtl '):
            if current_material and not map_kd_exists:
                # Add the map_Kd line before starting a new material
                map_kd_line = f"map_Kd {current_material}\n"
                new_lines.append(map_kd_line)
            
            current_material = line.split()[1]
            map_kd_exists = False
            new_lines.append(line)
        elif line.startswith('map_Kd '):
            map_kd_exists = True
            new_lines.append(line)
        else:
            new_lines.append(line)

    if current_material and not map_kd_exists:
        # Ensure the last material block has the map_Kd line
        map_kd_line = f"map_Kd {current_material}\n"
        new_lines.append(map_kd_line)

    with open(file_path, 'w') as file:
        file.writelines(new_lines)

def scan_folder_for_mtl_files(folder_path):
    for root, _, files in os.walk(folder_path):
        for file in files:
            if file.endswith('.mtl'):
                file_path = os.path.join(root, file)
                process_mtl_file(file_path)


if __name__ == "__main__":
    folder_path = "./"  # Change this to the path of your folder
    scan_folder_for_mtl_files(folder_path)
