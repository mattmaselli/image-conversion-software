const form = document.getElementById("convertForm");
const fileInput = document.getElementById("fileInput");
const fileInfo = document.getElementById("fileInfo");
const fileNameEl = document.getElementById("fileName");
const fileSizeEl = document.getElementById("fileSize");
const previewImage = document.getElementById("previewImage");
const formatSelect = document.getElementById("format");
const qualitySelect = document.getElementById("quality");
const qualityRow = document.getElementById("qualityRow");
const gifTimeRow = document.getElementById("gifTimeRow");
const gifTimeInput = document.getElementById("gifTime");
const gifLoopRow = document.getElementById("gifLoopRow");
const gifLoopInput = document.getElementById("gifLoop");
const convertButton = document.getElementById("convertButton");
const previewButton = document.getElementById("previewButton");
const downloadButton = document.getElementById("downloadButton");
const output = document.getElementById("output");

// upload text and hint for GIF vs non-GIF
const uploadText = document.querySelector(".upload-text");
const uploadHint = document.querySelector(".upload-hint");
const defaultUploadText = uploadText.textContent;
const defaultUploadHint = uploadHint.textContent;
const gifUploadText = "Click to choose images";
const gifUploadHint = "Hold Ctrl (or Cmd) to select multiple images.";

let selectedFile = null;
let convertedBlobUrl = null;
let lastFormat = formatSelect.value; // track previous format

function humanFileSize(bytes) {
  if (!bytes && bytes !== 0) return "";
  const units = ["B", "KB", "MB", "GB"];
  let i = 0;
  let value = bytes;
  while (value >= 1024 && i < units.length - 1) {
    value /= 1024;
    i++;
  }
  return value.toFixed(1) + " " + units[i];
}

function resetConvertedState() {
  if (convertedBlobUrl) {
    URL.revokeObjectURL(convertedBlobUrl);
    convertedBlobUrl = null;
  }
  previewButton.disabled = true;
  downloadButton.disabled = true;
}

function setStatus(message, isError) {
  output.textContent = message;
  output.style.color = isError ? "#f97373" : "#9ca3af";
}

// populate quality select 5 to 100 
(function initQualityOptions() {
  const step = 5;
  const min = 5;
  const max = 100;
  const defaultQuality = 80;
  for (let q = min; q <= max; q += step) {
    const option = document.createElement("option");
    option.value = q;
    option.textContent = q;
    if (q === defaultQuality) {
      option.selected = true;
    }
    qualitySelect.appendChild(option);
  }
})();

// toggle GIF-specific options + upload text + show/hide quality row
formatSelect.addEventListener("change", () => {
  resetConvertedState();

  const currentFormat = formatSelect.value;
  const isGif = currentFormat === "gif";

  // If we are switching FROM GIF to something else, clear the files + preview
  if (lastFormat === "gif" && !isGif) {
    fileInput.value = "";
    selectedFile = null;
    fileInfo.classList.add("hidden");
    previewImage.src = "";
    setStatus("", false);
  }

  // GIF-only options + upload messaging
  if (isGif) {
    gifTimeRow.classList.remove("hidden");
    gifLoopRow.classList.remove("hidden");
    uploadText.textContent = gifUploadText;
    uploadHint.textContent = gifUploadHint;
  } else {
    gifTimeRow.classList.add("hidden");
    gifLoopRow.classList.add("hidden");
    uploadText.textContent = defaultUploadText;
    uploadHint.textContent = defaultUploadHint;
  }

  // Completely hide quality row for GIF, PNG, TIFF
  const hideQuality =
    currentFormat === "gif" ||
    currentFormat === "png" ||
    currentFormat === "tiff";

  if (hideQuality) {
    qualityRow.classList.add("hidden");
  } else {
    qualityRow.classList.remove("hidden");
  }

  lastFormat = currentFormat;
});

fileInput.addEventListener("change", () => {
  const file = fileInput.files[0];
  if (!file) {
    selectedFile = null;
    fileInfo.classList.add("hidden");
    resetConvertedState();
    setStatus("", false);
    return;
  }
  if (!file.type.startsWith("image/")) {
    selectedFile = null;
    fileInfo.classList.add("hidden");
    resetConvertedState();
    setStatus("Please select an image file.", true);
    fileInput.value = "";
    return;
  }

  // still treat the first file as "selectedFile" for naming / preview
  selectedFile = file;
  resetConvertedState();

  fileNameEl.textContent = `File(s) selected: ${fileInput.files.length}`;
  fileSizeEl.textContent = `First file size: ${humanFileSize(file.size)}`;
  fileInfo.classList.remove("hidden");

  const reader = new FileReader();
  reader.onload = e => {
    previewImage.src = e.target.result;
  };
  reader.readAsDataURL(file);

  setStatus("Image(s) ready. Choose format and quality, then convert.", false);
});

form.addEventListener("submit", async e => {
  e.preventDefault();

  if (!fileInput.files || fileInput.files.length === 0) {
    setStatus("Please select at least one image.", true);
    return;
  }

  const format = formatSelect.value;

  resetConvertedState();
  convertButton.disabled = true;
  setStatus("Converting...", false);

  try {
    // GIF branch: multipart with multiple frame timing and loop
    if (format === "gif") {
      const delayMs = parseInt(gifTimeInput.value) || 100;
      const delayCs = Math.max(1, Math.round(delayMs / 10));

      const loopRaw = gifLoopInput.value.trim();
      const loopCount =
        loopRaw === "" ? 0 : Math.max(0, parseInt(loopRaw, 10) || 0);

      const formData = new FormData();
      for (const file of fileInput.files) {
        formData.append("frames", file);
      }
      formData.append("delay_cs", String(delayCs));
      formData.append("loop_count", String(loopCount));
      formData.append("target_w", "0");
      formData.append("target_h", "0");

      // assumes Rocket route is POST /convert/gif
      const resp = await fetch("/convert/gif", {
        method: "POST",
        body: formData
      });

      if (!resp.ok) {
        throw new Error("GIF conversion failed (" + resp.status + ")");
      }

      const blob = await resp.blob();
      convertedBlobUrl = URL.createObjectURL(blob);

      previewButton.disabled = false;
      downloadButton.disabled = false;
      setStatus("GIF conversion complete. Preview or download the image.", false);
      return;
    }

    // non-GIF formats: original single-file behavior
    if (!selectedFile) {
      setStatus("Please select an image file first.", true);
      return;
    }

    const quality = qualitySelect.value;
    const url = `/convert/${encodeURIComponent(format)}?quality=${encodeURIComponent(
      quality
    )}`;

    const resp = await fetch(url, {
      method: "POST",
      headers: { "Content-Type": selectedFile.type },
      body: selectedFile
    });

    if (!resp.ok) {
      throw new Error("Conversion failed (" + resp.status + ")");
    }

    const blob = await resp.blob();
    convertedBlobUrl = URL.createObjectURL(blob);

    previewButton.disabled = false;
    downloadButton.disabled = false;
    setStatus("Conversion complete. Preview or download the image.", false);
  } catch (err) {
    setStatus("Error: " + err.message, true);
  } finally {
    convertButton.disabled = false;
  }
});

previewButton.addEventListener("click", () => {
  if (!convertedBlobUrl) {
    setStatus("No converted image to preview yet.", true);
    return;
  }
  window.open(convertedBlobUrl, "_blank");
});

downloadButton.addEventListener("click", () => {
  if (!convertedBlobUrl || !selectedFile) {
    setStatus("No converted image to download yet.", true);
    return;
  }
  const format = formatSelect.value;
  const baseName = selectedFile.name.replace(/\.[^.]+$/, "");
  const ext = format === "jpg" ? "jpg" : format;
  const a = document.createElement("a");
  a.href = convertedBlobUrl;
  a.download = `${baseName}_converted.${ext}`;
  document.body.appendChild(a);
  a.click();
  a.remove();
});