;; Melpa
(require 'package) ;; You might already have this line
(add-to-list 'package-archives
             '("melpa" . "http://melpa.org/packages/") t)
(when (< emacs-major-version 24)
  ;; For important compatibility libraries like cl-lib
  (add-to-list 'package-archives '("gnu" . "http://elpa.gnu.org/packages/")))
(package-initialize) ;; You might already have this line

;; Correspondance des parenthèses :
;; Avec ceci, positionnez le curseur sur une parenthèse ouvrante ou
;; une parenthèse fermante, Emacs met en couleur la paire de
;; parenthèses.
(show-paren-mode 1)

;; Afficher les numéros de lignes dans la mode-line (barre du bas de
;; fenêtre) :
(line-number-mode t)
(column-number-mode t)

;; Ne pas afficher le message d'accueil
(setq inhibit-startup-message t)

;; Visionner la région (aka sélection) courante :
(transient-mark-mode t)

;; Correction orthographique :
(ispell-change-dictionary "francais")

;; Souligner les mots incorrects en mode LaTeX
(add-hook 'latex-mode-hook 'flyspell-mode)

;; Se limiter à des lignes de 80 caractères dans les modes textes (y
;; compris le mode LaTeX) :
;; cf. http://www-verimag.imag.fr/~moy/emacs/#autofill
(add-hook 'text-mode-hook 'turn-on-auto-fill)

;; Changer le comportement de la selection de fichiers (C-x C-f)
;; (ido-mode 1)

;; Pour une interface graphique un peu dépouillée
(menu-bar-mode 1)
(scroll-bar-mode 1)
(tool-bar-mode 1)

;; Définir des touches pour se déplacer rapidement :
;; Aller à la parenthèse ouvrante correspondante :
(global-set-key [M-right] 'forward-sexp)
;; Aller à la parenthèse Fermante correspondante :
(global-set-key [M-left] 'backward-sexp)

;; Compiler avec M-f9, recompiler (avec la même commande de
;; compilation) avec f9.
(global-set-key [M-f9]   'compile)
(global-set-key [f9]     'recompile)

;; Ouvrir les .txt en org-mode
(add-to-list 'auto-mode-alist '("\\.txt\\'" . org-mode))

;; Save Emacs session when exiting Emacs
(desktop-save-mode 1)

;; Disable right alt as meta
(setq ns-right-alternate-modifier nil)

;; Bind replace-string to C-x <F1>
(global-set-key (kbd "<f1>") 'replace-string)
(global-set-key [f2] 'shell)

;; auto-complete
(add-to-list 'load-path "~/.emacs.d/lisp")
(require 'auto-complete-config)
(add-to-list 'ac-dictionary-directories "~/.emacs.d/ac-dict")
(ac-config-default)
(auto-complete-mode 1)

;; Always show matching parenthesis and brackets
(show-paren-mode 1)

;; Tabbar change tab
(tabbar-mode 1)
(global-set-key [(super right)] 'tabbar-forward-tab)
(global-set-key [(super left)] 'tabbar-backward-tab)
(global-set-key [(super shift right)] 'tabbar-forward-group)
(global-set-key [(super shift left)] 'tabbar-backward-group)

;; "y or n" instead of "yes or no"
(fset 'yes-or-no-p 'y-or-n-p)

;; Don't truncate lines
(setq truncate-lines t)

;; Make sure all backup files only live in one place
(setq backup-directory-alist '(("." . "~/.emacs.d/backups")))

;; Trailing whitespace is unnecessary
(add-hook 'before-save-hook (lambda () (delete-trailing-whitespace)))

;; Window manipulation
(global-set-key [(super +)] 'enlarge-window)
(global-set-key [(super -)] 'shrink-window)

;; Tramp ssh (faster)
(setq tramp-default-method "ssh")

;; Linux coding style
(setq c-default-style "linux"
      c-basic-offset 3
      indent-tabs-mode nil)
