import ctypes
import sys
import winreg
import os

def is_admin():
    """Check if the script is running with administrative privileges."""
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

def run_as_admin():
    """Re-run the script with administrative privileges."""
    if sys.version_info[0] == 3 and sys.version_info[1] >= 9:
        executable = sys.executable
    else:
        executable = os.path.abspath(sys.argv[0])

    ctypes.windll.shell32.ShellExecuteW(None, "runas", executable, " ".join(sys.argv), None, 1)
    sys.exit()

def toggle_registry_value(key_path, value_name):
    """Toggle registry values between 0 and 1."""
    try:
        with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, key_path, 0, winreg.KEY_READ | winreg.KEY_WRITE) as key:
            value, reg_type = winreg.QueryValueEx(key, value_name)
            new_value = 0 if value == 1 else 1
            winreg.SetValueEx(key, value_name, 0, reg_type, new_value)
            print(f"Toggled {value_name} from {value} to {new_value}")
    except FileNotFoundError:
        print(f"Registry key or value {value_name} not found.")
    except PermissionError:
        print("Permission denied. Run the script as Administrator.")

if __name__ == "__main__":
    if not is_admin():
        print("Requesting Administrator privileges...")
        run_as_admin()

    registry_path = r"SYSTEM\CurrentControlSet\Control\PriorityControl"
    toggle_registry_value(registry_path, "SystemDockMode")
    toggle_registry_value(registry_path, "ConvertibleSlateMode")
