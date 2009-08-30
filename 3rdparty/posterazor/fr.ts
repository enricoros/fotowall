<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="fr">
<context>
    <name>Help</name>
    <message>
        <location filename="wizardcontroller.cpp" line="+174"/>
        <source>Step %1 of %2:</source>
        <translation type="unfinished">Étape %1 de %2:</translation>
    </message>
    <message>
        <location line="-69"/>
        <source>&amp;Manual</source>
        <translation type="unfinished">&amp;Manuel</translation>
    </message>
    <message>
        <location filename="controller.cpp" line="+540"/>
        <source>http://posterazor.sourceforge.net/</source>
        <comment>Only translate, if the website has this language.</comment>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="wizardcontroller.cpp" line="+78"/>
        <source>Load an input image</source>
        <translation type="unfinished">Ouvrir une image</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Printer paper format</source>
        <translation type="unfinished">Format du papier</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Image tile overlapping</source>
        <translation type="unfinished">Recouvrements de l&apos;image</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Final poster size</source>
        <translation type="unfinished">Taille final du poster</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Save the Poster</source>
        <translation type="unfinished">Enregistrer le Poster</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Load an image by clicking the button with the open icon and selecting an image file, or by drag &amp; dropping an image file on the PosteRazor. The drag &amp; drop also works during the other steps.
After loading the image, the most important informations are listed in the &lt;b&gt;%1&lt;/b&gt; fields.</source>
        <comment>Wizard step 1. Place holders: %1 = &apos;Image informations&apos; (will be automatically inserted)</comment>
        <translation type="unfinished">Chargez une image en cliquant sur l&apos;icône en forme de dossier. Choisissez le fichier en naviguant dans vos dossiers ou glissez-déposez l&apos;image dans la fenêtre. Le glissé-déposé marche aussi dans les autres étapes de l&apos;assistant.
Après avoir chargé l&apos;image, les informations pratiques (taille, couleur, résolution) sont affichées dans la rubrique &lt;b&gt;%1&lt;/b&gt;.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Define the paper sheet size that you use in your printer.
A standard paper sheet size can be selected from the &lt;b&gt;%1&lt;/b&gt; chooser, along with the desired paper sheet orientation.
Alternatively, a custom paper sheet size can be defined in the &lt;b&gt;%2&lt;/b&gt; tab.
Paper borders are defined in the &lt;b&gt;%3&lt;/b&gt; fields. Even if your printer does need no (or small) paper borders, some border might be needed to have enough area for gluing the final poster tiles together.</source>
        <comment>Wizard step 2. Place holders: %1 = &apos;Format:&apos;, %2 = &apos;Custom&apos;, %3 = &apos;Borders&apos; (will be automatically inserted)</comment>
        <translation type="unfinished">Dans cette étape, vous allez régler le type de papier que vous utiliserez pour imprimer.
Les formats de papier courants ainsi que l&apos;orientation (paysage/portrait) sont disponibles dans la catégorie &lt;b&gt;%1&lt;/b&gt;.
Cependant, si vous souhaitez définir vous-même la taille du papier, vous pouvez utiliser la catégorie &lt;b&gt;%2&lt;/b&gt;
&lt;b&gt;%3&lt;/b&gt;: Même si votre imprimante n&apos;impose pas de marges, il est toujours utile d&apos;en mettre, afin de permettre de coller les différents éléments du poster entre eux.</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Image tile overlapping is needed to have some tolerance for cutting off the unneeded borders from one side. Additionally, like the borders from the previous step, it gives more area for gluing together the final poster tiles.
The &lt;b&gt;%1&lt;/b&gt; defines the borders that are intended to be overlapped by the neighbor tiles. The borders on the opposite sides are intended to be cut (except on the outermost tiles).</source>
        <comment>Wizard step 3. Place holders: %1 = &apos;Overlapping position&apos; (will be automatically inserted)</comment>
        <translation type="unfinished">Les recouvrements entre morceaux donnent une certaine tolérance lors du découpage et du collage du poster.
&lt;b&gt;%1&lt;/b&gt; détermine les cotés qui seront recouverts.</translation>
    </message>
    <message>
        <location line="+9"/>
        <source>Define the final poster size, in one of the following three modes which can be selected by the corresponding radio buttons:</source>
        <comment>Wizard step 4. Start of the description.</comment>
        <translation type="unfinished">Pour déterminer la taille finale du poster, il existe trois méthodes :</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>You want to have a specific size of your poster.</source>
        <comment>Wizard step 4. Description for &apos;absolute size&apos;</comment>
        <translation type="unfinished">Vous spécifiez une taille précise pour votre poster.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>You want to use whole paper sheets and specify how many of them of them you want to use.</source>
        <comment>Wizard step 4. Description for &apos;size in pages&apos;</comment>
        <translation type="unfinished">La taille de votre poster dépend du nombre de pages que vous utiliserez.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Your input image has a certain size which is defined by the number of pixels and dpi (dots per Inch) and your want to enlarge the image by a certain factor.</source>
        <comment>Wizard step 4. Description for &apos;size in percent&apos;</comment>
        <translation type="unfinished">Vous choisissez un pourcentage de zoom en fonction de la résolution d&apos;origine de l&apos;image. La résolution d&apos;origine est calculée en fonction de la taille (hauteur / largeur) et du nombre de pixels par centimètres (ou DPI, nombre de pixels par pouces).</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>The aspect ratio of width and height is always 1:1 and is automatically recalculated. In the preview area, you can see the overlapping areas which are surrounded by light red rectangles.
&lt;b&gt;%1&lt;/b&gt; sets the alignment of the image on the total paper area of the poster. This is useful if you want to keep the unused paper.</source>
        <comment>Wizard step 4. End of the description. Place holders: %1 = &apos;Image alignment&apos; (will be automatically inserted)</comment>
        <translation type="unfinished">Par défaut, le poster est immédiatement calculé avec un rendu de 1 pour 1 (100%). Vous pouvez juger du rendu dans la fenêtre de prévisualisation. Les traits rouges figurent la coupure entre les pages.
Le bouton &lt;b&gt;%1&lt;/b&gt; permet de régler la position de l&apos;image par rapport au papier. L&apos;image peut être centrée ou collée sur un des bords.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Save the poster by clicking the save button and specifying a destination file name.
Check or uncheck the &lt;b&gt;%1&lt;/b&gt;, if the standard PDF handling application that is set in your operating system should be automatically started after the PDF file is saved.</source>
        <comment>Wizard step 5. Place holders: %1 = &apos;Open PDF after saving&apos; (will be automatically inserted)</comment>
        <translation type="unfinished">Enregistrez votre poster en cliquant sur le bouton en forme de disquette. Ensuite, donnez un nom à votre æuvre, choisissez l&apos;emplacement, puis cliquez sur Enregistrer.
Si vous cochez l&apos;option &lt;b&gt;%1&lt;/b&gt;, le poster sera automatiquement ouvert dans votre application PDF après l&apos;enregistrement.</translation>
    </message>
    <message>
        <location line="-144"/>
        <source>PosteRazor has its user interface organized in a &apos;Wizard&apos; fashion. All settings for the poster creation can be done in %1 steps.
The &lt;b&gt;%2&lt;/b&gt; and &lt;b&gt;%3&lt;/b&gt; buttons navigate through these steps. The &lt;b&gt;?&lt;/b&gt; button opens a help window with an explanation of the current step.
All entries and choices are remembered until the next usage of the PosteRazor.</source>
        <comment>Manual preface. Place holders: %1 = Number of wizard steps, %2 = &apos;Back&apos;, %3 = &apos;Next&apos; (will be automatically inserted)</comment>
        <translation type="unfinished">PosteRazor est organisé à la manière d&apos;un assistant. La création d&apos;un poster se fait en %1 étapes.
Les boutons &lt;b&gt;%2&lt;/b&gt; et &lt;b&gt;%3&lt;/b&gt; permettent de naviguer facilement entre ces différentes étapes. Le bouton &lt;b&gt;?&lt;/b&gt; ouvre une fenêtre d&apos;aide, qui explique l&apos;étape en cours.
Toutes les valeurs et choix sont conservés au prochain lancement de l&apos;application.</translation>
    </message>
</context>
<context>
    <name>Main window</name>
    <message>
        <location filename="wizard.cpp" line="+90"/>
        <location filename="wizardcontroller.cpp" line="+93"/>
        <source>Image Informations</source>
        <translation type="unfinished">Informations</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Color type:</source>
        <translation type="unfinished">Format :</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Resolution:</source>
        <translation type="unfinished">Résolution :</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Size (in pixels):</source>
        <translation type="unfinished">Taille (en pixels) :</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Input Image</source>
        <translation type="unfinished">Image à découper</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="wizardcontroller.cpp" line="+55"/>
        <source>Open PDF after saving</source>
        <translation type="unfinished">Ouvrir le PDF après avoir
sauvé le poster</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="wizardcontroller.cpp" line="-141"/>
        <source>Next</source>
        <translation type="unfinished">Suivant</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+12"/>
        <location line="+11"/>
        <location line="+4"/>
        <source>Height:</source>
        <translation type="unfinished">Hauteur :</translation>
    </message>
    <message>
        <location line="-26"/>
        <source>Bottom left</source>
        <translation type="unfinished">En bas à gauche</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Bottom right</source>
        <translation type="unfinished">En bas à droite</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="wizardcontroller.cpp" line="+104"/>
        <source>Overlapping position</source>
        <translation type="unfinished">Position du recouvrement</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Top left</source>
        <translation type="unfinished">En haut à gauche</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Top right</source>
        <translation type="unfinished">En haut à droite</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Overlapping size</source>
        <translation type="unfinished">Taille du recouvrement</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+6"/>
        <location line="+11"/>
        <location line="+5"/>
        <source>Width:</source>
        <translation type="unfinished">Largeur :</translation>
    </message>
    <message>
        <location line="-21"/>
        <source>Bottom</source>
        <translation type="unfinished">Bas</translation>
    </message>
    <message>
        <location line="+35"/>
        <source>Size (in %1):</source>
        <translation type="unfinished">Taille (en %1) :</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="wizardcontroller.cpp" line="-7"/>
        <source>Borders</source>
        <translation type="unfinished">Bordures</translation>
    </message>
    <message>
        <location line="-35"/>
        <source>Left</source>
        <translation type="unfinished">Gauche</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Right</source>
        <translation type="unfinished">Droite</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Top</source>
        <translation type="unfinished">Haut</translation>
    </message>
    <message>
        <location line="+3"/>
        <location filename="wizardcontroller.cpp" line="-2"/>
        <source>Format:</source>
        <translation type="unfinished">Format :</translation>
    </message>
    <message>
        <location line="+2"/>
        <location filename="wizardcontroller.cpp" line="+1"/>
        <source>Custom</source>
        <translation type="unfinished">Personnalisé</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Standard</source>
        <translation type="unfinished">Standard</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Orientation:</source>
        <translation type="unfinished">Orientation :</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Landscape</source>
        <translation type="unfinished">Paysage</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Portrait</source>
        <translation type="unfinished">Portrait</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Paper size</source>
        <translation type="unfinished">Format de papier</translation>
    </message>
    <message>
        <location line="+3"/>
        <location filename="wizardcontroller.cpp" line="+37"/>
        <source>Image alignment</source>
        <translation type="unfinished">Position de l&apos;image</translation>
    </message>
    <message>
        <location line="+1"/>
        <location line="+2"/>
        <source>pages</source>
        <translation type="unfinished">pages</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Size:</source>
        <translation type="unfinished">Taille :</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="wizardcontroller.cpp" line="-19"/>
        <source>Absolute size:</source>
        <translation type="unfinished">Taille réelle :</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Image size</source>
        <translation type="unfinished">Taille de l&apos;image</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="wizardcontroller.cpp" line="+5"/>
        <source>Size in pages:</source>
        <translation type="unfinished">Taille en nombres de pages :</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="wizardcontroller.cpp" line="+5"/>
        <source>Size in percent:</source>
        <translation type="unfinished">Taille en pourcentage :</translation>
    </message>
    <message>
        <location line="+1"/>
        <location filename="wizardcontroller.cpp" line="-125"/>
        <source>Back</source>
        <translation type="unfinished">Précédent</translation>
    </message>
    <message>
        <location filename="controller.cpp" line="-53"/>
        <location filename="wizard.cpp" line="+1"/>
        <source>Save the poster</source>
        <translation type="unfinished">Enregistrer le poster</translation>
    </message>
    <message>
        <location filename="wizard.cpp" line="+162"/>
        <source>Monochrome</source>
        <translation type="unfinished">Monochrome</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Gray scale</source>
        <translation type="unfinished">Niveau de gris</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Palette</source>
        <translation type="unfinished">Palette</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>RGB</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>RGBA</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+1"/>
        <source>CMYK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="controller.cpp" line="-70"/>
        <source>All image formats</source>
        <translation type="unfinished">Tous les formats d&apos;images</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Load an input image</source>
        <translation type="unfinished">Ouvrir une image</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>The image &apos;%1&apos; could not be loaded.</source>
        <translation type="unfinished">Le fichier &apos;%1&apos; ne peut pas être chargé.</translation>
    </message>
    <message>
        <location line="+55"/>
        <source>The file &apos;%1&apos; already exists.
Do you want to overwrite it?</source>
        <translation type="unfinished">Le fichier &apos;%1&apos; existe déjà.
Voulez-vous le remplacer ?</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>The file &apos;%1&apos; could not be saved.</source>
        <translation type="unfinished">Le fichier &apos;%1&apos; ne peut pas être enregistré.</translation>
    </message>
</context>
<context>
    <name>Wizard</name>
    <message>
        <location filename="wizard.ui"/>
        <source>Input Image</source>
        <translation type="unfinished">Image à découper</translation>
    </message>
    <message>
        <location/>
        <source>Image Informations</source>
        <translation type="unfinished">Informations</translation>
    </message>
    <message>
        <location/>
        <source>Size (in pixels):</source>
        <translation type="unfinished">Taille (en pixels) :</translation>
    </message>
    <message>
        <location/>
        <source>Size:</source>
        <translation type="unfinished">Taille :</translation>
    </message>
    <message>
        <location/>
        <source>Resolution:</source>
        <translation type="unfinished">Résolution :</translation>
    </message>
    <message>
        <location/>
        <source>Color type:</source>
        <translation type="unfinished">Format :</translation>
    </message>
    <message>
        <location/>
        <source>Paper size</source>
        <translation type="unfinished">Format de papier</translation>
    </message>
    <message>
        <location/>
        <source>Standard</source>
        <translation type="unfinished">Standard</translation>
    </message>
    <message>
        <location/>
        <source>Format:</source>
        <translation type="unfinished">Format :</translation>
    </message>
    <message>
        <location/>
        <source>Orientation:</source>
        <translation type="unfinished">Orientation :</translation>
    </message>
    <message>
        <location/>
        <source>Portrait</source>
        <translation type="unfinished">Portrait</translation>
    </message>
    <message>
        <location/>
        <source>Landscape</source>
        <translation type="unfinished">Paysage</translation>
    </message>
    <message>
        <location/>
        <source>Custom</source>
        <translation type="unfinished">Personnalisé</translation>
    </message>
    <message>
        <location/>
        <source>cm</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location/>
        <source>Width:</source>
        <translation type="unfinished">Largeur :</translation>
    </message>
    <message>
        <location/>
        <source>Height:</source>
        <translation type="unfinished">Hauteur :</translation>
    </message>
    <message>
        <location/>
        <source>Borders (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location/>
        <source>Top</source>
        <translation type="unfinished">Haut</translation>
    </message>
    <message>
        <location/>
        <source>Left</source>
        <translation type="unfinished">Gauche</translation>
    </message>
    <message>
        <location/>
        <source>Right</source>
        <translation type="unfinished">Droite</translation>
    </message>
    <message>
        <location/>
        <source>Bottom</source>
        <translation type="unfinished">Bas</translation>
    </message>
    <message>
        <location/>
        <source>Overlapping size</source>
        <translation type="unfinished">Taille du recouvrement</translation>
    </message>
    <message>
        <location/>
        <source>Overlapping position</source>
        <translation type="unfinished">Position du recouvrement</translation>
    </message>
    <message>
        <location/>
        <source>Top right</source>
        <translation type="unfinished">En haut à droite</translation>
    </message>
    <message>
        <location/>
        <source>Top left</source>
        <translation type="unfinished">En haut à gauche</translation>
    </message>
    <message>
        <location/>
        <source>Bottom left</source>
        <translation type="unfinished">En bas à gauche</translation>
    </message>
    <message>
        <location/>
        <source>Bottom right</source>
        <translation type="unfinished">En bas à droite</translation>
    </message>
    <message>
        <location/>
        <source>Poster size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location/>
        <source>pages</source>
        <translation type="unfinished">pages</translation>
    </message>
    <message>
        <location/>
        <source>Image size in percent</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location/>
        <source>Absolute image size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location/>
        <source>Size in pages</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location/>
        <source>%</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location/>
        <source>Image alignment</source>
        <translation type="unfinished">Position de l&apos;image</translation>
    </message>
    <message>
        <location/>
        <source>Save the poster</source>
        <translation type="unfinished">Enregistrer le poster</translation>
    </message>
    <message>
        <location/>
        <source>Open PDF after saving</source>
        <translation type="unfinished">Ouvrir le PDF après avoir
sauvé le poster</translation>
    </message>
    <message>
        <location/>
        <source>?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location/>
        <source>Step description</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location/>
        <source>Step x of y</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location/>
        <source>Back</source>
        <translation type="unfinished">Précédent</translation>
    </message>
    <message>
        <location/>
        <source>Next</source>
        <translation type="unfinished">Suivant</translation>
    </message>
</context>
</TS>
