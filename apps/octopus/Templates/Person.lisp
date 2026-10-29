(Button Height=56 Click="OpenPerson" BackgroundColor="#00000000" class="octopus-row"
  (Grid Columns="44px 1fr" Spacing=8 IgnoreHitTest=true
    (Grid Width=44 Height=44 StateClasses=(bind "DataContext/Tone") IgnoreHitTest=true
      (TextBlock Text=(bind "DataContext/Initials") FontSize=19 class="avatar-initials" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" IgnoreHitTest=true)
      (ImageView Name="AvatarPhoto" Visible=(bind "DataContext/HasAvatar") Src=(bind "DataContext/AvatarPath") ForegroundColor="#ffffff" Stretch="UniformToFill" IgnoreHitTest=true))
    (StackView Spacing=2 JustifyContent="Center" IgnoreHitTest=true
      (TextBlock Text=(bind "DataContext/Title") FontSize=20 TextOverflow="Ellipsis" TextWrapping="NoWrap" class="octopus-ink" IgnoreHitTest=true)
      (TextBlock Visible=(bind "DataContext/HasBio") Text=(bind "DataContext/BioExcerpt") FontSize=13 TextOverflow="Ellipsis" TextWrapping="NoWrap" class="octopus-dim" IgnoreHitTest=true))))
