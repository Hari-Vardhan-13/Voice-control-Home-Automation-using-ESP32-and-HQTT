# Deployment

## Live Demo
**URL:** https://home-automation-voice-control.netlify.app/

This is the production voice-control front end, built with Lovable and
deployed on Netlify. It replaces `web-app/index.html` (kept in this repo as
a lightweight local/offline reference implementation — see
`web-app/README.md`) as the version you'd actually demo or hand to a user.

## How it fits the project
```
 [Live app: home-automation-voice-control.netlify.app]   <- deployed, public HTTPS
         |  (speech -> text -> MQTT publish, over WebSocket+TLS)
         v
 [HiveMQ Cloud MQTT Broker]
         ^
         v
     [ESP32 firmware]  (firmware/VoiceControlHomeAutomation.ino)
```

Same broker, same firmware, same physical hardware — only the front end
changed from the local `index.html` prototype to a hosted app.

## Things to verify on the deployed app
Since the deployed app is a separate codebase (built in Lovable) rather than
`web-app/index.html`, these need to be checked/matched manually — see
"Changes for you to make" below.

## Redeploying
- **Netlify + Lovable:** push changes in the Lovable project editor; Lovable
  auto-syncs to the connected Netlify site, or trigger a manual redeploy
  from the Netlify dashboard.
- **Custom domain (optional):** Netlify site settings → Domain management,
  if you want something nicer than the `*.netlify.app` subdomain for a
  final submission/demo.
