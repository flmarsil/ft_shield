# Notes

- Ne doit se lancer qu'avec les droits root ✅ 
- En premier plan afficher le login sur la sortie standard ✅
- En arrière plan créer un binaire ft_shield dans le dossier contenant tous les binaires de l'OS et le lance. (/usr/bin) ✅
- Ce binaire doit se lancer automatiquement dès que la machine infectée s'allume (service) ✅
- Le programme de ce binaire doit être lancé en daemon ✅ 
- Une seule instance de ce daemon doit pouvoir être lancée ✅
- Le daemon doit écouter sur le port 4242 ✅
- Seulement 3 connexions clients simultanées sont autorisées ✅
- A chaque connexion un mot de passe est demandé (stockage du mot de passe de façon sécurisée : hash) ✅
- Lorsque la connexion est établie avec un client, le démon doit proposer de lancer un shell avec les droits root. Vous ne pourrez pas accéder à l'interpréteur de commandes avec la commande shell. ✅