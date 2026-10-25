; Standalone loader fixture; the Metro application remains unchanged.
(Screen Name="PivotDemo" Width=390 Height=844 ResizeMode="NoResize" ClearColor="#000000" StyleSheet="Metro/Screens/metro.css"
  (Grid Rows="1fr 58px" Foreground="#FFFFFF"
    (Pivot Name="Pivot" Title="Messenger"
      (PivotItem Name="Recent" Header="recent"
        (StackView Spacing=22
          (StackView Spacing=2
            (TextBlock class="item" Text="alex morgan")
            (TextBlock class="detail" Text="See you at the little coffee place?"))
          (StackView Spacing=2
            (TextBlock class="item" Text="weekend people")
            (TextBlock class="detail" Text="Maya: That sounds like a plan"))
          (StackView Spacing=2
            (TextBlock class="item" Text="jules park")
            (TextBlock class="detail" Text="The photos turned out so good"))))
      (PivotItem Name="Online" Header="online"
        (StackView Spacing=22
          (TextBlock class="item" Text="maya chen")
          (TextBlock class="item" Text="jules park")))
      (PivotItem Name="Groups" Header="groups"
        (TextBlock class="item" Text="weekend people")))
    (ApplicationBar Name="AppBar"
      (ApplicationBarIconButton Name="New" IconUri="Metro/Icons/plus.svg" Text="new")
      (ApplicationBarIconButton Name="Search" IconUri="Metro/Icons/search.svg" Text="search")
      (ApplicationBarIconButton Name="Refresh" IconUri="Metro/Icons/refresh-cw.svg" Text="refresh")
      (ApplicationBarMenuItem Name="Settings" Text="settings")
      (ApplicationBarMenuItem Name="About" Text="about"))))
