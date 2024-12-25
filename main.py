"""
Script to start the application
"""
from src.main import start_server


async def main():
    try:
        await start_server()
        print("Server started")
    except Exception as e:
        print(f"Failed to start server: {e}")


if __name__ == '__main__':
    import asyncio

    try:
        print("Starting server...")
        asyncio.run(main())
    except Exception as e:
        print(f"Failed to start server: {e}")
