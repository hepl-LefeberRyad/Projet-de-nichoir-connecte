<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>SmartCitiesIoT – Project: Nichoir</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; background: #f4f4f4; margin: 0; padding: 15px; }
        header { text-align: center; margin-bottom: 20px; }
        h1 { color: #333; margin: 0; font-size: 1.8rem; }
        #imageList { display: flex; flex-wrap: wrap; justify-content: center; gap: 15px; }
        .image-card { background: white; border-radius: 10px; padding: 10px; width: 250px; box-shadow: 0 0 10px rgba(0,0,0,0.08); cursor: pointer; }
        .image-card img { width: 100%; height: auto; border-radius: 6px; margin-top: 10px; }
        .info { font-size: 14px; color: #555; word-wrap: break-word; }

        #viewerModal { display: none; position: fixed; z-index: 9999; left: 0; top: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.85); justify-content: center; align-items: center; flex-direction: column; }
        #viewerImage { max-width: 95%; max-height: 75%; border-radius: 10px; }
        .arrow { font-size: 55px; color: white; cursor: pointer; user-select: none; padding: 10px; }
        #prevArrow { position: absolute; left: 10px; top: 50%; transform: translateY(-50%); }
        #nextArrow { position: absolute; right: 10px; top: 50%; transform: translateY(-50%); }
        #closeBtn { position: absolute; top: 15px; right: 20px; font-size: 35px; color: white; cursor: pointer; padding: 10px; }

        @media (max-width: 600px) {
            .arrow { font-size: 65px; }
            #viewerImage { max-width: 95%; max-height: 70%; }
            h1 { font-size: 1.5rem; }
            .image-card { width: 90%; }
        }
    </style>
</head>
<body>

<header>
    <h1>SmartCitiesIoT</h1>
    <h2>Project: Nichoir</h2>
</header>

<div id="imageList">
    {% for img in images %}
        <div class="image-card" onclick="openViewer({{ loop.index0 }})">
            <div class="info">
                <strong>ID:</strong> {{ img.id }}<br>
                <strong>Topic:</strong> {{ img.topic }}<br>
                <strong>Timestamp:</strong> {{ img.timestamp }}
            </div>
            <img src="data:image/jpeg;base64,{{ img.img_b64 }}" alt="MQTT Image">
        </div>
    {% endfor %}
</div>

<div id="viewerModal">
    <span id="closeBtn" onclick="closeViewer()">×</span>
    <span id="prevArrow" class="arrow" onclick="prevImage()">‹</span>
    <img id="viewerImage" src="">
    <span id="nextArrow" class="arrow" onclick="nextImage()">›</span>
</div>

<script>
    let images = [];
    let currentIndex = 0;

    function rebuildImageList() {
        images = [];
        const cards = document.querySelectorAll('#imageList .image-card img');
        cards.forEach(img => images.push(img.src));
    }

    function openViewer(index) {
        currentIndex = index;
        document.getElementById("viewerImage").src = images[currentIndex];
        document.getElementById("viewerModal").style.display = "flex";
    }

    function closeViewer() {
        document.getElementById("viewerModal").style.display = "none";
    }

    function nextImage() {
        currentIndex = (currentIndex + 1) % images.length;
        document.getElementById("viewerImage").src = images[currentIndex];
    }

    function prevImage() {
        currentIndex = (currentIndex - 1 + images.length) % images.length;
        document.getElementById("viewerImage").src = images[currentIndex];
    }

    async function refreshContent() {
        try {
            const response = await fetch(window.location.href, { cache: "no-store" });
            const html = await response.text();
            const parser = new DOMParser();
            const doc = parser.parseFromString(html, "text/html");

            const updatedList = doc.getElementById("imageList");
            document.getElementById("imageList").innerHTML = updatedList.innerHTML;

            rebuildImageList();
        } catch (err) {
            console.error("Auto-refresh error:", err);
        }
    }

    setInterval(refreshContent, 5000);
    rebuildImageList();
</script>

</body>
</html>
