import asyncio
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


async def handle_client(reader, writer):
    addr = writer.get_extra_info('peername')
    print(f"Connected by {addr}")

    try:
        while True:
            data = await reader.read(1024)

            message = data.decode()
            request = json.loads(message)
            print(f"Received from C socket: {request}")
            action = request.get("action")
            response = {"foo": "bar"}

            await send_to_client_in_chunks(writer, response)
            # json_data = json.dumps(response, cls=CustomJSONEncoder)
            # writer.write(json_data.encode())
            # await writer.drain()

    except Exception as e:
        response = {"status": "error", "message": str(e)}
        await send_to_client_in_chunks(writer, response)
        # writer.write(json.dumps(response).encode())
        # await writer.drain()

    # writer.close()
    # await writer.wait_closed()
