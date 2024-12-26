"""
Entry point for initializing the app components
"""
import asyncio

from src.handlers.handler import handle_client
from src.settings import Settings


async def start_server():
    # asyncio.create_task(monitor_rooms())

    server = await asyncio.start_server(
        handle_client, Settings.HOST, Settings.PORT
    )

    addr = server.sockets[0].getsockname()
    print(f'Serving on {addr}')
    # Create tables

    async with server:
        await server.serve_forever()
