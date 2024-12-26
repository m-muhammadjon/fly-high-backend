import json
import struct

CHUNK_SIZE = 16384


async def send_to_client_in_chunks(writer, data, chunk_size=CHUNK_SIZE):
    """
    Sends data to the client in chunks with a length header.
    """
    json_data = json.dumps(data).encode('utf-8')
    total_length = len(json_data)

    # Send the total length as a 4-byte header
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
    return {"status": "success", "message": "Received data"}


async def handle_client(reader, writer):
    addr = writer.get_extra_info('peername')
    print(f"Connected by {addr}")

    try:
        while True:
            raw_data = await reader.read(1024)
            raw_data = raw_data.decode()
            try:
                data = json.loads(raw_data)
                print(f"{data=}")
                # request, authorization, details = await parse_data(data)
                # print("1")
                # response = await get_response(request, authorization, details)
                print(f"Received from C socket: {data}")

                await send_to_client_in_chunks(writer, {"foo": "bar"})
                # json_data = json.dumps(response, cls=CustomJSONEncoder)
                # writer.write(json_data.encode())
                # await writer.drain()
            except json.JSONDecodeError:
                print("Error: Invalid JSON data received")
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
