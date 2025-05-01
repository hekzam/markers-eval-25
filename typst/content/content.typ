#import "../components/container.typ": container
#let bcol = json("../style/input-fill-colors.json")

// Contenu de la copie d'examen.
#let content = [
  #set align(left+top)

  = Exercise 1
  #container("b0", 5cm, 1cm, fill-color: color.rgb(bcol.at("b0")))
  #box(container("b0i", 8cm, 2.5cm, fill-color: color.rgb(bcol.at("b0i"))))
  #container("b1", 100%, 1cm, fill-color: color.rgb(bcol.at("b1")))

  = Exercise 2
  #container("b2", 5cm, 1cm, fill-color: color.rgb(bcol.at("b2")))
  #box(container("b2i", 8cm, 2.5cm, fill-color: color.rgb(bcol.at("b2i"))))

  = Exercise 3
  This exercise should be at the bottom of its page.
  #container("b3", 100%, 1cm, fill-color: color.rgb(bcol.at("b3")))

  = Exercice 4: Code Imaginaire
  This is placeholder code: 1234567890

  = Exercice 5: Autres placeholders
  Here is some dummy code:
  #box("def placeholder():\n    print(\"Placeholder function executed\")\n    return \"result\"\n\nprint(placeholder())")
]