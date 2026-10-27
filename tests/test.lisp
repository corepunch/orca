(Screen Name="test-screen" Width=1000 Height=1000 ResizeMode="NoResize"
  (TextBlock Name="test-text" FontSize=20 Text="This is a test screen.")
  ; The grid derives its height from the inner stack's two fixed-height nodes.
  (StackView Name="outer-stack" Direction="Vertical"
    (Grid Name="test-grid" Columns="1fr 1fr 1fr"
      (StackView Name="inner-stack" Direction="Vertical"
        (Node2D Name="node1" Height=50)
        (Node2D Name="node2" Height=50))))
  ; The container includes both the text height and its padding.
  (Node2D Name="metric-users" Width=140 Padding=20
    (TextBlock Name="metric-text" Text="2.4 M" FontSize=28)))
