<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8"> <!-- Encodage des caractères -->
    <title>SmartCitiesIoT – Project: Nichoir</title> <!-- Titre affiché dans l'onglet du navigateur -->
    <meta name="viewport" content="width=device-width, initial-scale=1"> <!-- Adaptation pour les mobiles -->

    <style>
        /* Styles généraux */
        body { font-family: Arial, sans-serif; background: #f4f4f4; margin: 0; padding: 15px; }
        header { text-align: center; margin-bottom: 20px; }
        h1 { color: #333; margin: 0; font-size: 1.8rem; }
        h2 { color: #666; font-weight: normal; margin-top: 5px; font-size: 1rem; }

        /* Section batterie */
        #batteryStatus { margin-top: 10px; text-align: center; font-size: 1rem; color: #333; }

        /* Liste des images */
        #imageList { display: flex; flex-wrap: wrap; justify-content: center; gap: 15px; }
        .image-card { background: white; border-radius: 10px; padding: 10px; width: 250px; box-shadow: 0 0 10px rgba(0,0,0,0.08); cursor: pointer; }
        .image-card img { width: 100%; height: auto; border-radius: 6px; margin-top: 10px; }
        .info { font-size: 14px; color: #555; word-wrap: break-word; }

        /* Modal pour visualiser l'image en grand */
        #viewerModal { 
            display: none; /* Masqué par défaut */
            position: fixed; 
            z-index: 9999; 
            left: 0; top: 0; 
            width: 100%; height: 100%; 
            background: rgba(0,0,0,0.85); 
            justify-content: center; 
            align-items: center; 
            flex-direction: column; 
        }
        #viewerImage { max-width: 95%; max-height: 75%; border-radius: 10px; }
        .arrow { font-size: 55px; color: white; cursor: pointer; user-select: none; padding: 10px; }
        #prevArrow { position: absolute; left: 10px; top: 50%; transform: translateY(-50%); }
        #nextArrow { position: absolute; right: 10px; top: 50%; transform: translateY(-50%); }
        #closeBtn { position: absolute; top: 15px; right: 20px; font-size: 35px; color: white; cursor: pointer; padding: 10px; }

        /* Adaptation pour petits écrans */
        @media (max-width: 600px) {
            .arrow { font-size: 65px; }
            #viewerImage { max-width: 95%; max-height: 70%; }
            h1 { font-size: 1.5rem; }
            h2 { font-size: 0.9rem; }
            .image-card { width: 90%; }
        }
    </style>
</head>

<body>

<header>
    <h1>SmartCitiesIoT</h1> <!-- Titre principal -->
    <h2>Project: Nichoir</h2> <!-- Sous-titre -->

    <!-- Section affichage batterie -->
    <div id="batteryStatus">
        {% if battery.voltage is not none and battery.capacity is not none and battery.timestamp is not none %}
            <!-- Si les infos de batterie existent, les afficher -->
            <strong>Battery Voltage:</strong> {{ "%.2f"|format(battery.voltage) }} V &nbsp; | &nbsp;
            <strong>Capacity:</strong> {{ battery.capacity }} % &nbsp; | &nbsp;
            <strong>Updated:</strong> {{ battery.timestamp }}
        {% else %}
            <!-- Sinon message par défaut -->
            <em>No battery data yet</em>
        {% endif %}
    </div>
</header>

<!-- Liste des images -->
<div id="imageList">
    {% for img in images %}
        <div class="image-card" onclick="openViewer({{ loop.index0 }})"> <!-- Ouvre le modal au clic -->
            <div class="info">
                <strong>ID:</strong> {{ img.id }}<br>
                <strong>Topic:</strong> {{ img.topic }}<br>
                <strong>Timestamp:</strong> {{ img.timestamp }}
            </div>
            <img src="data:image/jpeg;base64,{{ img.img_b64 }}" alt="MQTT Image"> <!-- Affiche l'image -->
        </div>
    {% endfor %}
</div>

<!-- Modal pour visualisation en grand -->
<div id="viewerModal">
    <span id="closeBtn" onclick="closeViewer()">✖</span> <!-- Bouton fermer -->
    <span id="prevArrow" class="arrow" onclick="prevImage()">⯇</span> <!-- Flèche précédente -->
    <img id="viewerImage" src=""> <!-- Image affichée en grand -->
    <span id="nextArrow" class="arrow" onclick="nextImage()">⯈</span> <!-- Flèche suivante -->
</div>

<script>
    let images = [];        // Tableau pour stocker les sources des images
    let currentIndex = 0;   // Index de l'image affichée dans le modal

    // Reconstruit le tableau images à partir du DOM
    function rebuildImageList() {
        images = [];
        const cards = document.querySelectorAll('#imageList .image-card img');
        cards.forEach(img => images.push(img.src));
    }

    // Ouvre le modal sur l'image index
    function openViewer(index) {
        currentIndex = index;
        document.getElementById("viewerImage").src = images[currentIndex];
        document.getElementById("viewerModal").style.display = "flex";
    }

    // Ferme le modal
    function closeViewer() {
        document.getElementById("viewerModal").style.display = "none";
    }

    // Affiche l'image suivante dans le modal
    function nextImage() {
        currentIndex = (currentIndex + 1) % images.length;
        document.getElementById("viewerImage").src = images[currentIndex];
    }

    // Affiche l'image précédente dans le modal
    function prevImage() {
        currentIndex = (currentIndex - 1 + images.length) % images.length;
        document.getElementById("viewerImage").src = images[currentIndex];
    }

    /* Auto-refresh toutes les 5 secondes */
    async function refreshContent() {
        try {
            const response = await fetch(window.location.href, { cache: "no-store" }); // Récupère le HTML
            const html = await response.text();                                         // Transforme en texte
            const parser = new DOMParser();                                             // Parseur HTML
            const doc = parser.parseFromString(html, "text/html");

            // Met à jour la liste d'images
            const updatedList = doc.getElementById("imageList");
            document.getElementById("imageList").innerHTML = updatedList.innerHTML;

            // Met à jour les infos batterie
            const updatedBattery = doc.getElementById("batteryStatus");
            document.getElementById("batteryStatus").innerHTML = updatedBattery.innerHTML;

            // Reconstruit le tableau images pour le modal
            rebuildImageList();
        } catch (err) {
            console.error("Auto-refresh error:", err); // Affiche les erreurs éventuelles
        }
    }

    setInterval(refreshContent, 5000); // Actualisation automatique toutes les 5 secondes

    // Construction initiale du tableau images
    rebuildImageList();
</script>

</body>
</html>
