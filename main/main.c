#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#ifdef CONFIG_BT_NIMBLE_ENABLED
#include "esp_nimble_hci.h"
#endif

#include "driver/gpio.h"

static const char *TAG = "BLE_HID_GAMEPAD";

static const uint8_t hid_report_map[] __attribute__((unused)) = {
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x04,       // Usage (Joystick)
    0xA1, 0x01,       // Collection (Application)
      // Buttons
      0x05, 0x09,     // Usage Page (Button)
      0x19, 0x01,     // Usage Minimum (Button 1)
      0x29, 0x08,     // Usage Maximum (Button 8)
      0x15, 0x00,     // Logical Minimum (0)
      0x25, 0x01,     // Logical Maximum (1)
      0x95, 0x08,     // Report Count (8)
      0x75, 0x01,     // Report Size (1)
      0x81, 0x02,     // Input (Data,Var,Abs)
      // X and Y axes
      0x05, 0x01,     // Usage Page (Generic Desktop)
      0x09, 0x30,     // Usage (X)
      0x09, 0x31,     // Usage (Y)
      0x15, 0x81,     // Logical Min (-127)
      0x25, 0x7F,     // Logical Max (127)
      0x75, 0x08,     // Report Size (8)
      0x95, 0x02,     // Report Count (2)
      0x81, 0x02,     // Input (Data,Var,Abs)
    0xC0              // End Collection
};

// --- GPIO pin mapping ---
#define BUTTON_BOOT_PIN      0   // BOOT no devkit -> usado como R2 virtual
#define BUTTON_TRIANGLE_PIN  13
#define BUTTON_CIRCLE_PIN    12
#define BUTTON_CROSS_PIN     14
#define BUTTON_SQUARE_PIN    27

#define GPIO_DEBOUNCE_MS 50

typedef struct {
    uint8_t buttons;
    int8_t  x;
    int8_t  y;
} __attribute__((packed)) hid_report_t;

typedef enum { EVT_BTN_PRESS, EVT_BTN_RELEASE } btn_event_type_t;
typedef struct {
    uint8_t btn_mask;
    btn_event_type_t type;
    uint8_t gpio_num;
} btn_event_t;

static QueueHandle_t btn_queue = NULL;

#define BTN_TRIANGLE_MASK  (1<<0)
#define BTN_R2_MASK        (1<<1)
#define BTN_CROSS_MASK     (1<<2)
#define BTN_SQUARE_MASK    (1<<3)

static hid_report_t current_report = {0, 0, 0};

// NimBLE prototypes
static int ble_app_gap_event(struct ble_gap_event *event, void *arg);
static void ble_app_on_sync(void);
static void host_task(void *param);

static uint16_t conn_handle = 0xffff;
static uint16_t hid_input_report_handle = 0; // preenchido pelo GATT quando adicionado

// --- send HID report as notification ---
static void send_hid_report_to_host(hid_report_t *rpt) {
    if (conn_handle == 0xffff) {
        ESP_LOGI(TAG, "Nenhum cliente conectado - não envio");
        return;
    }
    if (hid_input_report_handle == 0) {
        ESP_LOGW(TAG, "Handle de input report nao definido ainda");
        return;
    }

    struct os_mbuf *om = ble_hs_mbuf_from_flat(rpt, sizeof(hid_report_t));
    if (om == NULL) {
        ESP_LOGW(TAG, "Falha ao alocar mbuf para notify");
        return;
    }

    int rc = ble_gattc_notify_custom(conn_handle, hid_input_report_handle, om);
    if (rc != 0) {
        ESP_LOGW(TAG, "Falha ao notificar host rc=%d", rc);
    } else {
        ESP_LOGI(TAG, "Relatório enviado: buttons=0x%02x X=%d Y=%d", rpt->buttons, rpt->x, rpt->y);
    }
}

// --- ISR and button task ---
static void IRAM_ATTR gpio_isr_handler(void *arg) {
    uint32_t gpio = (uint32_t) arg;
    uint8_t btn_mask = 0;
    if (gpio == BUTTON_TRIANGLE_PIN) btn_mask = BTN_TRIANGLE_MASK;
    else if (gpio == BUTTON_BOOT_PIN) btn_mask = BTN_R2_MASK;
    else if (gpio == BUTTON_CROSS_PIN) btn_mask = BTN_CROSS_MASK;
    else if (gpio == BUTTON_SQUARE_PIN) btn_mask = BTN_SQUARE_MASK;

    int level = gpio_get_level(gpio); // active-low assumed
    btn_event_t e;
    e.gpio_num = gpio;
    e.btn_mask = btn_mask;
    e.type = (level == 0) ? EVT_BTN_PRESS : EVT_BTN_RELEASE;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(btn_queue, &e, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
}

static void btn_task(void *arg) {
    btn_event_t ev;
    for (;;) {
        if (xQueueReceive(btn_queue, &ev, portMAX_DELAY) == pdTRUE) {
            vTaskDelay(pdMS_TO_TICKS(GPIO_DEBOUNCE_MS));
            int level = gpio_get_level(ev.gpio_num);
            btn_event_type_t real_type = (level == 0) ? EVT_BTN_PRESS : EVT_BTN_RELEASE;
            if (real_type != ev.type) continue;

            if (ev.type == EVT_BTN_PRESS) current_report.buttons |= ev.btn_mask;
            else current_report.buttons &= ~ev.btn_mask;

            const char *name = "UNKNOWN";
            if (ev.btn_mask == BTN_TRIANGLE_MASK) name = "Triangulo";
            else if (ev.btn_mask == BTN_R2_MASK) name = "R2";
            else if (ev.btn_mask == BTN_CROSS_MASK) name = "X";
            else if (ev.btn_mask == BTN_SQUARE_MASK) name = "Quadrado";

            ESP_LOGI(TAG, "%s %s (mask=0x%02x)", name, (ev.type==EVT_BTN_PRESS) ? "pressionado" : "solto", ev.btn_mask);

            send_hid_report_to_host(&current_report);
        }
    }
}

// ---------------- GATT definitions ----------------
static ble_uuid16_t hid_svc_uuid = BLE_UUID16_INIT(0x1812);

static struct ble_gatt_chr_def hid_chr_defs[] = {
    {
        .uuid = BLE_UUID16_DECLARE(0x2A4D), /* HID Report */
        .access_cb = NULL,
        .val_handle = &hid_input_report_handle,
        .flags = BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_READ,
    },
    { 0 }
};

static struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &hid_svc_uuid.u,
        .characteristics = hid_chr_defs,
    },
    { 0 }
};

// ---------------- GAP callbacks ----------------
static int ble_app_gap_event(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            conn_handle = event->connect.conn_handle;
            ESP_LOGI(TAG, "Conectado conn_handle=%d", conn_handle);
        } else {
            ESP_LOGI(TAG, "Falha ao conectar; status=%d", event->connect.status);
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "Desconectado; reason=%d", event->disconnect.reason);
        conn_handle = 0xffff;
        ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER,
                         &(struct ble_gap_adv_params){0}, NULL, NULL);
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "Advertising complete");
        break;
    default:
        break;
    }
    return 0;
}

// ---------------- on_sync: register services and start advertising ----------------
static void ble_app_on_sync(void)
{
    int rc;

    const char *name = "ESP-Controle";
    ble_svc_gap_device_name_set(name);

    ble_svc_gap_init();
    ble_svc_gatt_init();

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_count_cfg falhou rc=%d", rc);
    } else {
        rc = ble_gatts_add_svcs(gatt_svr_svcs);
        if (rc != 0) {
            ESP_LOGE(TAG, "ble_gatts_add_svcs falhou rc=%d", rc);
        } else {
            ESP_LOGI(TAG, "GATT services registrados OK");
        }
    }

    ESP_LOGI(TAG, "hid_input_report_handle = %u", hid_input_report_handle);

    /* Configure advertisement fields: Flags + Complete Local Name + TX Power */
    {
        struct ble_hs_adv_fields fields;
        const char *device_name = ble_svc_gap_device_name();
        memset(&fields, 0, sizeof(fields));

        fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
        fields.tx_pwr_lvl_is_present = 1;
        fields.tx_pwr_lvl = 0;

        if (device_name) {
            fields.name = (uint8_t *)device_name;
            fields.name_len = strlen(device_name);
            fields.name_is_complete = 1;
        }

        int r = ble_gap_adv_set_fields(&fields);
        if (r != 0) {
            ESP_LOGE(TAG, "ble_gap_adv_set_fields rc=%d", r);
        } else {
            ESP_LOGI(TAG, "adv fields set ok (name='%s')", device_name ? device_name : "(null)");
        }
    }

    /* advertising params */
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    /* intervals: 0x20 -> ~40ms, 0x40 -> ~80ms (good for discovery) */
    adv_params.itvl_min = 0x20;
    adv_params.itvl_max = 0x40;

    rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params, ble_app_gap_event, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gap_adv_start falhou rc=%d", rc);
    } else {
        ESP_LOGI(TAG, "BLE sincronizado e advertising iniciado com nome '%s'", name);
    }
}

// ---------------- host task ----------------
static void host_task(void *param) {
    ESP_LOGI(TAG, "NimBLE host task start");
    nimble_port_run();
    nimble_port_freertos_deinit();
}

// ---------------- init BLE ----------------
static void ble_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

#ifdef CONFIG_BT_NIMBLE_ENABLED
    ret = esp_nimble_hci_and_controller_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_nimble_hci_and_controller_init failed: %d", ret);
        return;
    }
#endif

    nimble_port_init();

    ble_hs_cfg.reset_cb = NULL;
    ble_hs_cfg.sync_cb = ble_app_on_sync;

    nimble_port_freertos_init(host_task);
}

// ---------------- application main ----------------
void app_main(void) {
    ESP_LOGI(TAG, "Inicializando BLE HID Gamepad (template)");

    btn_queue = xQueueCreate(16, sizeof(btn_event_t));
    xTaskCreate(btn_task, "btn_task", 4096, NULL, 8, NULL);

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = ((1ULL<<BUTTON_BOOT_PIN) | (1ULL<<BUTTON_TRIANGLE_PIN) | (1ULL<<BUTTON_CROSS_PIN) | (1ULL<<BUTTON_SQUARE_PIN)),
        .pull_down_en = 0,
        .pull_up_en = 1
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_BOOT_PIN, gpio_isr_handler, (void*) BUTTON_BOOT_PIN);
    gpio_isr_handler_add(BUTTON_TRIANGLE_PIN, gpio_isr_handler, (void*) BUTTON_TRIANGLE_PIN);
    gpio_isr_handler_add(BUTTON_CROSS_PIN, gpio_isr_handler, (void*) BUTTON_CROSS_PIN);
    gpio_isr_handler_add(BUTTON_SQUARE_PIN, gpio_isr_handler, (void*) BUTTON_SQUARE_PIN);

    ble_init();

    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_LOGI(TAG, "Setup concluído. Conecte pelo Bluetooth do PC ao dispositivo 'ESP-Controle'");
}
