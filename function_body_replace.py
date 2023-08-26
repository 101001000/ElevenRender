import sys
import re
from os import walk

# I will provide a directory with 0.ll, 1.ll, 2.ll ...

if len(sys.argv) != 3:
    print("Usage: python replace_markers.py <shader_dir> <shader_placeholder.ll>")
    sys.exit(1)


shader_dir, shader_placeholder = sys.argv[1:3]

filenames = next(walk(shader_dir), (None, None, []))[2]
filenames = sorted(filenames)

def addrspace2fptr(content):  
    spirv_adapt = content.replace("ptr", "float*")
    cuda_adapt = spirv_adapt.replace("addrspace(4)", "")
    ocl_adapt = cuda_adapt.replace("float* addrspace(2)", "ptr addrspace(2)")
    return ocl_adapt

def remove_metadata(content):
    no_number_metadata = re.sub('![0-9]+', " ", content, flags=re.DOTALL)
    no_loop_metadata = no_number_metadata.replace(", !llvm.loop", "")
    no_tbaa_metadata = no_loop_metadata.replace(", !tbaa", "")
    return no_tbaa_metadata

for filename in filenames:
    
    print("Opening ", filename)

    with open(shader_dir + filename, 'r') as f:
    
        content = f.read()
        content = remove_metadata(content)
        
        reg_res = re.finditer('(entry:)(.*?)(})', content, flags=re.DOTALL)
        for r in reg_res:
            extracted_body = r.group()
                
        with open(shader_placeholder, 'r') as f:
            placeholder_content = f.read()
            placeholder_content = addrspace2fptr(placeholder_content)
           
        id = int(filename.replace(".ll", ""))
           
        print("ID: ", id)
        print("EXTRACTED BODY:", extracted_body)
           
        pattern = 'define((?:(?!define).)+?)(asl_shade' + str(id) + '_)(.+?){.+?}'
        function_definitions = re.finditer(pattern, placeholder_content, flags=re.DOTALL)
        for match in function_definitions :
            
            print("MATCH ", match.group())
            
            modified_content = re.sub('(entry:)(.*?)(})', extracted_body, match.group(), flags=re.DOTALL)
            
            print("MODIFIED CONTENT: ", modified_content)
            
            placeholder_content = placeholder_content.replace(match.group(), modified_content)
            
           
        with open(shader_placeholder, 'w') as f:
            f.write(placeholder_content)
    