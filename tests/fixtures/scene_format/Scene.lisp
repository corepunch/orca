(Node2D Name="FormatScene" Width=100 Height=80
  DataContext=(SceneFormatDraft Name="Context" Title="draft")
  (TextBlock Name="Label" Text="A & B <tag> \"quoted\"\nПривет")
  (Input Name="Edit" Text=(bind "DataContext/Title" Mode=TwoWay))
  (Node2D Name="Computed" Width=(bind "../Width")
    Height=(if (step 90 (bind "../Width")) 40 20))
  (Node2D Name="Target" Visible=true)
  (Node2D Name="Hide" LeftButtonUp=(HideAction Path="../Target")))
