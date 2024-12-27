from src.database import cursor, DB
from src.utils import get_user_id

async def handle_add_flight(token, data):
    representative_id = await get_user_id(token)

    cursor.execute("SELECT COALESCE(MAX(flight_id), 0) AS last_flight_id FROM flight")
    result = cursor.fetchone()
    last_flight_id = result[0]
    new_flight_id = last_flight_id + 1

    cursor.execute(
        """
        INSERT INTO flight (
            flight_id, airline_id, plane_id, includes_food, from_airport, start_date,
            start_date_time_zone, to_airport, is_international_flight, end_date, 
            end_date_time_zone, refund, baggage_per_person, carry_on_baggage_per_person, 
            is_exchangeable_for_additional_fee, state
        )
        VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
        """,
        (
            new_flight_id,
            1,
            data.get("plane_id"),
            data.get("includes_food", 0),
            data.get("from_airport"),
            data.get("start_date"),
            data.get("start_date_time_zone"),
            data.get("to_airport"),
            data.get("is_international_flight", 0),
            data.get("end_date"),
            data.get("end_date_time_zone"),
            data.get("refund"),
            data.get("baggage_per_person"),
            data.get("carry_on_baggage_per_person"),
            data.get("is_exchangeable_for_additional_fee", 0),
            data.get("state")
        )
    )

    # Commit the transaction to save changes
    DB.commit()

    return {"status": "SUCCESS", "message": "Flight added successfully."}