#ifndef COMMON_JS_H
#define COMMON_JS_H

const char *common_js = R"rawliteral(
<script>
function showAlert(message, type = 'info') {
  const container = document.getElementById('alert-container');
  const alertClass = type === 'success' ? 'alert-success'
                     : type === 'error' ? 'alert-error' : 'alert-warning';
  container.innerHTML = `<div class="alert ${alertClass}">${message}</div>`;
  setTimeout(() => { container.innerHTML = ''; }, 5000);
}
</script>
)rawliteral";

#endif // COMMON_JS_H
