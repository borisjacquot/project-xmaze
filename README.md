```


 __   ____  __           ___________ _   _  _____ 
 \ \ / /  \/  |   /\    |___  /_   _| \ | |/ ____|
  \ V /| \  / |  /  \      / /  | | |  \| | |  __ 
   > < | |\/| | / /\ \    / /   | | | . ` | | |_ |
  / . \| |  | |/ ____ \  / /__ _| |_| |\  | |__| |
 /_/ \_\_|  |_/_/    \_\/_____|_____|_| \_|\_____|
                                                  
                                                  

```

(contraction de XMAZE et AMAZING :)

# XMAZING

Un projet réalisé par Boris Jacquot et Louis Wadbled.


## Installation

### Dépendances

+ SDL2

```bash
apt install libsdl2-dev
apt install libsdl2-gfx-dev
```

### Compilation

```bash
make clean all
```

### Execution

```bash
./server/server     // lancer le serveur
./client/client     // lancer le client
```

## Fonctionnalités

# Serveur

+ ✅ - Broadcast UDP
+ ✅ - Connexion TCP avec le serveur
+ ✅ - Récepetion et execution des commandes
+ ✅ - Réception des touches
+ ✅ - Mise à jour de la position du joueur

+ ⚠️ - Envoi des graphismes ➡️ *ne semble fonctionner que sur la machine locale, et n'envoie que des murs*

+ 🆘 - Gestion des spheres ➡️ *fonctions prêtes sauf le tri fusion qui ne fonctionne pas, donc pas d'envoi de disques au client*

# Client

todo

## Auteurs

+ **Boris Jacquot** - *server*
+ **Louis Wadbled** - *client*
+ **Xavier Redon** - *interface graphique + labyrinthe*

## License

![WTFPL](http://www.wtfpl.net/wp-content/uploads/2012/12/wtfpl-badge-2.png)