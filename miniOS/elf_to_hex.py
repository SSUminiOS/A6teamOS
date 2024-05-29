# -*- coding: utf-8 -*-

def hex_string_to_c_array(hex_string):
    # 16진수 문자열을 두 글자씩 분리하여 리스트 생성
    hex_bytes = [hex_string[i:i+2] for i in range(0, len(hex_string), 2)]
    
    # C언어 스타일의 바이트 배열로 변환
    formatted_bytes = ', '.join("0x{}".format(byte) for byte in hex_bytes)
    
    # 완성된 문자열을 반환
    return "{{ {} }};".format(formatted_bytes)

def read_from_file(file_path):
    # 파일 읽기
    with open(file_path, 'r') as file:
        return file.read().strip()

def write_to_file(file_path, data):
    # 파일 쓰기
    with open(file_path, 'w') as file:
        file.write(data)

# 입력 파일과 출력 파일 경로 설정
input_file_path = 'input_hex.txt'
output_file_path = 'output_array.txt'

# 입력 파일에서 16진수 문자열 읽기
input_hex_string = read_from_file(input_file_path)

# 16진수 문자열을 C언어 스타일의 바이트 배열로 변환
result_array = hex_string_to_c_array(input_hex_string)

# 결과를 출력 파일에 쓰기
write_to_file(output_file_path, result_array)

print(f"Converted hex array has been written to {output_file_path}")