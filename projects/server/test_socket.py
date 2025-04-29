import socket
import os
import json
import argparse

def send_message(json_message) -> dict:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect(('localhost', 9092))
        s.sendall(json.dumps(json_message).encode() + b'\n')
        response = s.recv(1024)
        return response.decode()

def test(test_dict):
    pack_number = 1
    for pack in test_dict:
        if 'request' not in pack:
            print(f'Error: request not found. Pack {pack_number}')
            return False
        if 'result' not in pack:
            print(f'Error: result not found. Pack {pack_number}')
            return False
        result = json.loads(send_message(pack['request']))
        for key, value in pack['result'].items():
            if key not in result:
                print(f'Error: no such {key} in results. Pack {pack_number}')
                return False
            if result[key] != value:
                print(f'Error: Values of {key} not compare. Value {result[key]} must be {value}. Pack {pack_number}')
                return False
        pack_number += 1
    print('Success')
    return True

def test_process(path):
    print(f'Start Test {len(os.listdir(path))} files')
    success = True
    for test_file in os.listdir(path):
        print('Test ', test_file, end=' ')
        with open(os.path.join(path,test_file), 'r') as file:
            loaded_test = json.load(file)
        success = success and test(loaded_test)
    if success:
        print('Test completed successfully')
    else:
        print('Tests Failed')

def create_message() -> dict:
    print('Creating message...')
    msg_dict = dict()
    msg_dict['server_id'] = int(input('(Required)Write server Id: '))
    msg_dict['command'] = int(input('(Required)Write command Id: '))
    print('The Folowing parameters are optional. To finish message type 'f'. To skip parameter just press Enter.')
    optional_int_parameters = {
        'player_id': ['(Optional)Write Player Id: ', 'int'],
        'game_id': ['(Optional)Write Game Id: ', 'int'],
        'computer_id': ['(Optional)Write Computer Id: ', 'int'],
        'step': ['(Optional)Write step: ', 'int'],
        'game_value': ['(Optional)Write game value: ', 'int'],
        'game_brain': ['(Optional)Write computer level [Easiest,Easy,Medium,Hard]: ', 'str']
    }
    for name, value in optional_int_parameters.items():
        text = input(value[0])
        if text == '':
            continue
        if text == 'f':
            break
        if value[1] == 'int':
            msg_dict[name] = int(text)
        else:
            msg_dict[name] = text
    print(f'Message created: {msg_dict}')
    return msg_dict

def construct_command( number ) -> dict:
    command_dict = dict()
    command_dict["number"] = number
    command_dict["request"] = create_message()
    command_dict["result"] = json.loads(send_message(command_dict["request"]))
    print(f'Recived: {command_dict["result"]}')
    return command_dict

def construct_test():
    test_folder = '.generated_tests'
    test_name = input('Name the test: ')
    if not test_name.endswith('.json'):
        test_name += '.json'
    test_commands = []
    while True:
        test_commands.append(construct_command(len(test_commands) + 1))
        msg = input('Add another command?[y/N]: ')
        if msg != 'y' and msg != 'yes':
            break
    with open(os.path.join(test_folder,test_name), 'w') as f:
        f.write(json.dumps(test_commands, ensure_ascii=False, indent=4))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Script for testing Bulls And Cows server')
    parser.add_argument('-t', action='store_true', help='Start Tests in test folder')
    parser.add_argument('-b', action='store_true', help='Self made tests mode')
    args = parser.parse_args()
    if args.t:
        print('Start testing a Bulls And Cows Server')
        test_process('./tests/')
    elif args.b:
        print('Start self made tests')
        while True:
            construct_test()
            msg = input('Contstruct another test?[y/N]')
            if msg != 'y' and msg != 'yes':
                break
    else:
        print('Start free server interaction mode')
        while(True):
            msg_dict = create_message()
            result = send_message(msg_dict)
            print(f'Recived: {result}')