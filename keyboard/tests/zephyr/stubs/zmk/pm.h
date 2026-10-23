// Frontière matérielle : les périphériques ne sont pas éteints par le test.
#pragma once
int zmk_pm_suspend_devices(void);
void zmk_pm_resume_devices(void);
