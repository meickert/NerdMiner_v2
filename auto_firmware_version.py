import subprocess

Import("env")

def get_firmware_specifier_build_flag():
    try:
        ret = subprocess.run(["git", "describe"], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True) #Uses only annotated tags
        #ret = subprocess.run(["git", "describe", "--tags"], stdout=subprocess.PIPE, text=True) #Uses any tags
        build_version = ret.stdout.strip()
    except OSError:
        build_version = ""
    if not build_version:
        build_version = "unknown"  # Fallback: kein git / keine annotierten Tags (z.B. Fork ohne Tags)
    # fix unwanted and verbose tags
    build_version = build_version.replace('Release', '')
    build_flag = "-D AUTO_VERSION=\\\"" + build_version + "\\\""
    print ("Firmware Revision: " + build_version)
    return (build_flag)

env.Append(
    BUILD_FLAGS=[get_firmware_specifier_build_flag()]
)
