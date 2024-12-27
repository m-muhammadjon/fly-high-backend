import asyncio
import json
import struct

from . import auth

CHUNK_SIZE = 16384


async def send_to_client_in_chunks(writer, data, chunk_size=CHUNK_SIZE):
    """
    Sends data to the client in chunks with a length header.
    """
    json_data = json.dumps(data).encode('utf-8')
    total_length = len(json_data)

    # Send the total length as a 4-byte header
    print(f"SEND TO CLIENT: {json_data}")
    # await asyncio.sleep(3)
    writer.write(struct.pack('!I', total_length))
    await writer.drain()

    # Send data in larger chunks
    for i in range(0, total_length, chunk_size):
        chunk = json_data[i:i + chunk_size]
        writer.write(chunk)
        await writer.drain()


async def parse_data(data):
    """
    Parses the data received from the client.
    """
    return data.get("request"), data.get("authorization"), data.get("details")


async def get_response(request, authorization, details):
    match request:
        case "login-user":
            return await auth.handle_login_user(details)
        case "register-user":
            return await auth.handle_register_user(details)
        case _:
            return {"status": "error", "message": "Invalid request"}


async def handle_client(reader, writer):
    addr = writer.get_extra_info('peername')
    print(f"Connected by {addr}")

    try:
        while True:
            raw_data = await reader.read(1024)
            raw_data = raw_data.decode()
            try:
                print(f"{raw_data=}")
                data = json.loads(raw_data)
                print(f"{data=}")
                request, authorization, details = await parse_data(data)
                print(f"{request=}")
                # print("1")
                response = await get_response(request, authorization, details)
                print(f"Received from C socket: {data}")

                await send_to_client_in_chunks(writer, response)
                # json_data = json.dumps(response, cls=CustomJSONEncoder)
                # writer.write(json_data.encode())
                # await writer.drain()
            except Exception as e:
                print("Error: Invalid JSON data received")
                print(e)
                response = {"status": "error", "message": "Invalid JSON data received"}
                await send_to_client_in_chunks(writer, response)
                # writer.write(json.dumps(response).encode())
                # await writer.drain()

    except Exception as e:
        print(f"Error: {e}")
        response = {"status": "error", "message": str(e)}
        await send_to_client_in_chunks(writer, response)
        # writer.write(json.dumps(response).encode())
        # await writer.drain()

    # writer.close()
    # await writer.wait_closed()
