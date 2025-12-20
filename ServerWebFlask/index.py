<!DOCTYPE html>
<html>
<head>
    <!-- Set the title of the browser tab -->
    <title>MQTT Messages</title>

    <!-- Internal CSS styles -->
    <style>
        /* Style for the table element */
        table {
            width: 70%;               /* Table width is 70% of the page */
            margin: 20px auto;        /* Center the table with top/bottom margin 20px */
            border-collapse: collapse; /* Collapse borders so that they are single lines */
        }

        /* Apply border to table, table header (th) and table data (td) */
        table, th, td {
            border: 1px solid #555;  /* Dark gray border */
        }

        /* Style for table header and data cells */
        th, td {
            padding: 8px;             /* Padding inside cells */
            text-align: center;       /* Center-align the text */
        }

        /* Style for the entire page body */
        body {
            font-family: Arial;       /* Use Arial font */
            background-color: #f4f4f4; /* Light gray background */
            text-align: center;       /* Center text by default */
        }

        /* Style for the main heading */
        h1 {
            margin-top: 30px;         /* Add top margin */
        }
    </style>
</head>
<body>
    <!-- Main heading of the page -->
    <h1>Messages Received from MQTT</h1>

    <!-- Table to display MQTT messages -->
    <table>
        <tr>
            <!-- Table header row -->
            <th>ID</th>        <!-- Column for message ID -->
            <th>Topic</th>     <!-- Column for MQTT topic -->
            <th>Payload</th>   <!-- Column for MQTT message payload -->
            <th>Timestamp</th> <!-- Column for timestamp -->
        </tr>

        <!-- Loop over each message passed from Flask -->
        {% for msg in messages %}
        <tr>
            <!-- Display each field of the message in its respective cell -->
            <td>{{ msg[0] }}</td>  <!-- Message ID -->
            <td>{{ msg[1] }}</td>  <!-- Topic -->
            <td>{{ msg[2] }}</td>  <!-- Payload -->
            <td>{{ msg[3] }}</td>  <!-- Timestamp -->
        </tr>
        {% endfor %}
    </table>
</body>
</html>
