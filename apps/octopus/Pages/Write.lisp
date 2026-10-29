(StackView Name="WritePage" Spacing=14 Overflow="hidden scroll" JustifyContent="Start"
  (StackView Name="WriteSignIn" Visible=(bind "DataContext/Reader/IsGuest") Spacing=16 JustifyContent="Start"
    (StackView Spacing=8 Padding="0 0 24 0" JustifyContent="Start"
      (TextBlock Text="sign in to write" FontSize=22 WordWrap=true class="octopus-muted" IgnoreHitTest=true)
      (TextBlock Text="Guests can read everything. Writing, replying and following need an account." FontSize=14 WordWrap=true class="octopus-dim" IgnoreHitTest=true))
    (Button Name="WriteSignInButton" Click="SignInToWrite" Height=48 FontSize=15 FontWeight="Bold" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" class="octopus-action octopus-primary" "Sign in")
    (Button Name="WriteSignUp" Click="SignUpToWrite" Height=44 BorderWidth=2 FontSize=15 FontWeight="Bold" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" class="octopus-action octopus-outline" "Create an account"))
  (StackView Name="WriteForm" Visible=(bind "DataContext/Reader/SignedIn") Spacing=14 JustifyContent="Start"
    (Grid Name="PostKinds" Columns="1fr 1fr" Rows="80px 80px" Spacing=8 Height=168
      (Button Name="Kindtext" Click="SelectKind" Height=80 StateClasses=(bind "DataContext/Composer/TextTile") class="octopus-action kind-tile"
        (StackView Spacing=4 JustifyContent="Center" IgnoreHitTest=true
          (VectorView Source="Octopus/assets/icons/file-text.svg" Width=28 Height=28 MarginLeft="auto" MarginRight="auto" class="kind-ink" IgnoreHitTest=true)
          (TextBlock Text="text" FontSize=17 TextHorizontalAlignment="Center" class="kind-ink" IgnoreHitTest=true)))
      (Button Name="Kindphoto" Click="SelectKind" Height=80 StateClasses=(bind "DataContext/Composer/PhotoTile") class="octopus-action kind-tile"
        (StackView Spacing=4 JustifyContent="Center" IgnoreHitTest=true
          (VectorView Source="Octopus/assets/icons/image.svg" Width=28 Height=28 MarginLeft="auto" MarginRight="auto" class="kind-ink" IgnoreHitTest=true)
          (TextBlock Text="photo" FontSize=17 TextHorizontalAlignment="Center" class="kind-ink" IgnoreHitTest=true)))
      (Button Name="Kindquote" Click="SelectKind" Height=80 StateClasses=(bind "DataContext/Composer/QuoteTile") class="octopus-action kind-tile"
        (StackView Spacing=4 JustifyContent="Center" IgnoreHitTest=true
          (VectorView Source="Octopus/assets/icons/quote.svg" Width=28 Height=28 MarginLeft="auto" MarginRight="auto" class="kind-ink" IgnoreHitTest=true)
          (TextBlock Text="quote" FontSize=17 TextHorizontalAlignment="Center" class="kind-ink" IgnoreHitTest=true)))
      (Button Name="Kindlink" Click="SelectKind" Height=80 StateClasses=(bind "DataContext/Composer/LinkTile") class="octopus-action kind-tile"
        (StackView Spacing=4 JustifyContent="Center" IgnoreHitTest=true
          (VectorView Source="Octopus/assets/icons/link.svg" Width=28 Height=28 MarginLeft="auto" MarginRight="auto" class="kind-ink" IgnoreHitTest=true)
          (TextBlock Text="link" FontSize=17 TextHorizontalAlignment="Center" class="kind-ink" IgnoreHitTest=true))))
    (StackView Visible=(bind "DataContext/Composer/IsText") Spacing=6 JustifyContent="Start"
      (TextBlock Text="title" FontSize=14 class="octopus-muted" IgnoreHitTest=true)
      (Input Name="PostTitle" PlaceholderText="Give it a title" Height=44 FontSize=15 Padding="12 0" BorderWidth=2 class="octopus-input octopus-field" Text=(bind "DataContext/Draft/Title" Mode=TwoWay)))
    (StackView Visible=(bind "DataContext/Composer/IsText") Spacing=6 JustifyContent="Start"
      (TextBlock Text="post" FontSize=14 class="octopus-muted" IgnoreHitTest=true)
      (Input Name="PostContent" PlaceholderText="What's on your mind? Markdown works on the website." Multiline=true Height=176 FontSize=15 Padding="12 10" BorderWidth=2 class="octopus-input octopus-field" Text=(bind "DataContext/Draft/Content" Mode=TwoWay)))
    (StackView Visible=(bind "DataContext/Composer/IsQuote") Spacing=6 JustifyContent="Start"
      (TextBlock Text="quote" FontSize=14 class="octopus-muted" IgnoreHitTest=true)
      (Input Name="QuoteText" PlaceholderText="The words worth sharing" Multiline=true Height=106 FontSize=15 Padding="12 10" BorderWidth=2 class="octopus-input octopus-field" Text=(bind "DataContext/Draft/Content" Mode=TwoWay)))
    (StackView Visible=(bind "DataContext/Composer/IsQuote") Spacing=6 JustifyContent="Start"
      (TextBlock Text="source (optional)" FontSize=14 class="octopus-muted" IgnoreHitTest=true)
      (Input Name="QuoteSource" PlaceholderText="Who said it? A person, book or site" Height=44 FontSize=15 Padding="12 0" BorderWidth=2 class="octopus-input octopus-field" Text=(bind "DataContext/Draft/QuoteSource" Mode=TwoWay)))
    (StackView Visible=(bind "DataContext/Composer/IsLink") Spacing=6 JustifyContent="Start"
      (TextBlock Text="title" FontSize=14 class="octopus-muted" IgnoreHitTest=true)
      (Input Name="LinkTitle" PlaceholderText="What is it?" Height=44 FontSize=15 Padding="12 0" BorderWidth=2 class="octopus-input octopus-field" Text=(bind "DataContext/Draft/Title" Mode=TwoWay)))
    (StackView Visible=(bind "DataContext/Composer/IsLink") Spacing=6 JustifyContent="Start"
      (TextBlock Text="address" FontSize=14 class="octopus-muted" IgnoreHitTest=true)
      (Input Name="LinkUrl" PlaceholderText="https://" Height=44 FontSize=15 Padding="12 0" BorderWidth=2 class="octopus-input octopus-field" Text=(bind "DataContext/Draft/LinkUrl" Mode=TwoWay)))
    (StackView Visible=(bind "DataContext/Composer/IsLink") Spacing=6 JustifyContent="Start"
      (TextBlock Text="description (optional)" FontSize=14 class="octopus-muted" IgnoreHitTest=true)
      (Input Name="LinkDescription" PlaceholderText="Why it's worth a read" Multiline=true Height=92 FontSize=15 Padding="12 10" BorderWidth=2 class="octopus-input octopus-field" Text=(bind "DataContext/Draft/Content" Mode=TwoWay)))
    (StackView Name="PhotoField" Visible=(bind "DataContext/Composer/TakesPhoto") Spacing=8 JustifyContent="Start"
      (TextBlock Text=(bind "DataContext/Composer/PhotoLabel") FontSize=14 class="octopus-muted" IgnoreHitTest=true)
      (Grid Name="PhotoPreview" Height=200 class="octopus-raised" IgnoreHitTest=true Visible=(and (bind "DataContext/Composer/IsPhoto") (bind "DataContext/Draft/HasPhoto"))
        (ImageView Src=(bind "DataContext/Draft/PhotoPath") Stretch="Uniform" ForegroundColor="#ffffff" IgnoreHitTest=true))
      (Grid Name="QuotePhotoPreview" Height=200 class="octopus-raised" IgnoreHitTest=true Visible=(and (bind "DataContext/Composer/IsQuote") (bind "DataContext/Draft/HasPhoto"))
        (ImageView Src=(bind "DataContext/Draft/PhotoPath") Stretch="Uniform" ForegroundColor="#ffffff" IgnoreHitTest=true))
      (Button Name="ChoosePhoto" Click="ChoosePhoto" Height=44 BorderWidth=2 FontSize=15 FontWeight="Bold" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" IgnoreHitTest=(bind "DataContext/Draft/Status/IsSubmitting") class="octopus-action octopus-outline" Text=(if (bind "DataContext/Draft/HasPhoto") "Choose another photo" "Choose a photo"))
      (Button Name="RemovePhoto" Visible=(bind "DataContext/Draft/HasPhoto") Click="RemovePhoto" Height=48 FontSize=15 TextHorizontalAlignment="Left" TextVerticalAlignment="Center" IgnoreHitTest=(bind "DataContext/Draft/Status/IsSubmitting") class="octopus-action octopus-link" "Remove photo"))
    (StackView Visible=(bind "DataContext/Composer/IsPhoto") Spacing=6 JustifyContent="Start"
      (TextBlock Text="caption (optional)" FontSize=14 class="octopus-muted" IgnoreHitTest=true)
      (Input Name="PhotoCaption" PlaceholderText="Say something about it" Multiline=true Height=76 FontSize=15 Padding="12 10" BorderWidth=2 class="octopus-input octopus-field" Text=(bind "DataContext/Draft/Content" Mode=TwoWay)))
    (StackView Visible=(bind "DataContext/Composer/IsQuote") Spacing=6 JustifyContent="Start"
      (TextBlock Text="your thoughts (optional)" FontSize=14 class="octopus-muted" IgnoreHitTest=true)
      (Input Name="QuoteThoughts" PlaceholderText="What it means to you" Multiline=true Height=76 FontSize=15 Padding="12 10" BorderWidth=2 class="octopus-input octopus-field" Text=(bind "DataContext/Draft/UserText" Mode=TwoWay)))
    (StackView Spacing=6 JustifyContent="Start"
      (TextBlock Text="tags (optional)" FontSize=14 class="octopus-muted" IgnoreHitTest=true)
      (Input Name="PostTags" PlaceholderText="design, reef, hello" Height=44 FontSize=15 Padding="12 0" BorderWidth=2 class="octopus-input octopus-field" Text=(bind "DataContext/Draft/Tags" Mode=TwoWay)))
    (Button Name="Publish" Click="Draft.Submit" Height=48 FontSize=15 FontWeight="Bold" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" IgnoreHitTest=(bind "DataContext/Draft/Status/IsSubmitting") class="octopus-action octopus-primary" Text=(if (bind "DataContext/Draft/Status/IsSubmitting") "Publishing…" "Publish") Opacity=(if (bind "DataContext/Draft/Status/IsSubmitting") 0.5 1))
    (TextBlock Name="WriteStatus" FontSize=13 WordWrap=true class="octopus-dim")))
